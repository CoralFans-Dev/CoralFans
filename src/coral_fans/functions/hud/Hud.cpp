#include "coral_fans/functions/hud/Hud.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/data/Data.h"
#include "coral_fans/functions/func/FuncManager.h"

#include "ll/api/base/StdInt.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/common/Brightness.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/TextPacketType.h"
#include "mc/util/ProfilerLite.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/phys/HitResultType.h"
#include <algorithm>
#include <map>
#include <string>


namespace coral_fans::functions {

std::vector<std::pair<std::string, uint64>> HudHelper::HudTypeVec = {
    {"mspt",      HudType::mspt     },
    {"base",      HudType::base     },
    {"redstone",  HudType::redstone },
    {"village",   HudType::village  },
    {"hopper",    HudType::hopper   },
    {"block",     HudType::block    },
    {"container", HudType::container}
};

void HudHelper::tick() {
    using ll::i18n_literals::operator""_tr;
    static int gt;
    auto&      mod = coral_fans::mod();
    if (gt == 1) { // delay 1 tick to get village info
        auto level = ll::service::getLevel();
        if (level) {
            level->forEachPlayer([&](Player& player) {
                if (mod.getConfigDb()->get("functions.players." + player.getUuid().asString() + ".cfhud.show")
                    == "false")
                    return true;
                unsigned long hud;
                try {
                    hud = std::stoul(mod.getConfigDb()
                                         ->get("functions.players." + player.getUuid().asString() + ".cfhud.hud")
                                         .value_or("0"));
                } catch (...) {
                    return true;
                }
                std::string msg;
                double mspt = (double)ProfilerLite::gProfilerLiteInstance().mDebugServerTickTime->count() / 1000000.0;
                double currentTps  = mspt <= 50 ? 20 : (double)(1000.0 / mspt);
                auto   hitrst      = player.traceRay(5.25f, true, true);
                auto&  blockSource = player.getDimensionBlockSource();
                if (hud & (1 << HudHelper::HudType::mspt)) {
                    msg += std::format(
                        "MSPT:{}{:.2f}§r TPS:{}{:.2f}§r\n",
                        mspt > 50 ? "§c" : "§a",
                        mspt,
                        currentTps < 20 ? "§c" : "§a",
                        currentTps
                    );
                }
                if (hud & (1 << HudHelper::HudType::base)) {
                    auto& delta  = player.getPosDelta();
                    auto& biome  = blockSource.getBiome(player.getFeetBlockPos());
                    msg         += "translate.cfhud.base"_tr(
                        level->getCurrentServerTick().tickID,
                        player.getPosition().toString(),
                        player.getViewVector(1.0f).toString(),
                        utils::blockPosToChunkPos(player.getFeetBlockPos()).toString(),
                        hitrst.mType == HitResultType::Tile ? hitrst.mBlock.toString() : "-",
                        hitrst.mType == HitResultType::Tile ? std::to_string(blockSource
                                                                                 ._getRawBrightness(
                                                                                     hitrst.mBlock + BlockPos{0, 1, 0},
                                                                                     player.getDimension().mSkyDarken,
                                                                                     true,
                                                                                     true
                                                                                 )
                                                                                 .mValue)
                                                                    : "-",
                        delta.length() * 20,
                        delta.x * 20,
                        delta.y * 20,
                        delta.z * 20,
                        magic_enum::enum_name(biome.getBiomeType())
                    );
                }
                if (hud & (1 << HudHelper::HudType::redstone)) {
                    if (hitrst.mType == HitResultType::Tile) {
                        auto rst  = showRedstoneComponentsInfo(player.getDimension(), hitrst.mBlock, 2);
                        msg      += rst.first + "\n";
                    }
                }
                if (hud & (1 << HudHelper::HudType::village)) {
                    if (hitrst.mType == HitResultType::Entity) {
                        auto* entity = hitrst.getEntity();
                        if (entity) {
                            auto rst  = mod.getVillageManager().getVillagerInfo(entity->getOrCreateUniqueID());
                            msg      += rst.first + "\n";
                        }
                    }
                }
                if (hud & (1 << HudHelper::HudType::hopper)) {
                    int ch = HopperCounterManager::getViewChannel(blockSource, hitrst);
                    if (ch != -1) msg += mod.getHopperCounterManager().getChannel(ch).info() + "\n";
                }
                if (hud & (1 << HudHelper::HudType::block)) {
                    if (hitrst.mType == HitResultType::Tile) {
                        auto rst  = getBlockData(blockSource, hitrst.mBlock);
                        msg      += rst + "\n";
                    }
                }
                if (hud & (1 << HudHelper::HudType::container)) {
                    if (hitrst.mType == HitResultType::Tile) {
                        const auto& bl = blockSource.getBlock(hitrst.mBlock);
                        auto*       ba = blockSource.getBlockEntity(hitrst.mBlock);
                        if (bl.mLegacyBlock->isContainerBlock() && ba) {
                            auto* container = ba->getContainer();
                            if (container) {
                                std::map<std::string, int> items;
                                for (const auto& item : container->getSlots()) {
                                    if (*item == ItemStack::EMPTY_ITEM()) continue;
                                    std::string name = item->getCustomName();
                                    if (name.empty()) name = item->getName();
                                    if (auto it = items.find(name); it != items.end()) it->second += item->mCount;
                                    else items.emplace(name, item->mCount);
                                }
                                std::vector<std::pair<std::string, int>> vec(items.begin(), items.end());
                                std::sort(
                                    vec.begin(),
                                    vec.end(),
                                    [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                                        if (a.second == b.second) return a.first < b.first;
                                        else return a.second > b.second;
                                    }
                                );
                                for (const auto& it : vec) msg += std::format("{} : {}\n", it.first, it.second);
                            }
                        }
                    }
                }
                if (msg.ends_with('\n')) msg = msg.substr(0, msg.length() - 1);
                if (!msg.empty()) {
                    auto pkt     = TextPacket();
                    pkt.mType    = TextPacketType::Tip;
                    pkt.mMessage = msg;
                    player.sendNetworkPacket(pkt);
                }
                return true;
            });
        }
    }
    gt = (gt + 1) % std::max(2, mod.getConfig().cfhudRefreshTime);
}

} // namespace coral_fans::functions