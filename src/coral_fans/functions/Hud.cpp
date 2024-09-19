#include "coral_fans/functions/Hud.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/Data.h"
#include "coral_fans/functions/HopperCounter.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/enums/TextPacketType.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/util/ProfilerLite.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/Biome.h"

namespace coral_fans::functions {

void HudHelper::tick() {
    using ll::i18n_literals::operator""_tr;
    static int gt;
    if (gt == 1) { // delay 1 tick to get village info
        auto level = ll::service::getLevel();
        if (level) {
            level->forEachPlayer([&](Player& player) {
                auto& mod = coral_fans::mod();
                if (mod.getConfigDb()->get("functions.players." + player.getUuid().asString() + ".cfhud.show")
                    != "true")
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
                double      mspt = (double)ProfilerLite::gProfilerLiteInstance.getServerTickTime().count() / 1000000.0;
                double      currentTps  = mspt <= 50 ? 20 : (double)(1000.0 / mspt);
                auto        hitrst      = player.traceRay(5.25f, true, true);
                auto&       blockSource = player.getDimensionBlockSource();
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
                    auto& delta  = player.getPosDeltaNonConst();
                    auto& biome  = blockSource.getBiome(player.getFeetBlockPos());
                    msg         += "translate.cfhud.base"_tr(
                        level->getCurrentServerTick().t,
                        player.getPosition().toString(),
                        player.getViewVector(1.0f).toString(),
                        utils::blockPosToChunkPos(player.getFeetBlockPos()).toString(),
                        hitrst.mBlockPos.toString(),
                        blockSource.getRawBrightness(hitrst.mBlockPos + BlockPos{0, 1, 0}, true, true).value,
                        delta.length() * 20,
                        delta.x * 20,
                        delta.y * 20,
                        delta.z * 20,
                        magic_enum::enum_name(biome.getBiomeType())
                    );
                }
                if (hud & (1 << HudHelper::HudType::redstone)) {
                    auto rst  = showRedstoneComponentsInfo(player.getDimension(), hitrst.mBlockPos, 2);
                    msg      += rst.first + "\n";
                }
                if (hud & (1 << HudHelper::HudType::village)) {
                    auto* entity = hitrst.getEntity();
                    if (entity) {
                        auto rst  = mod.getVillageManager().getVillagerInfo(entity->getOrCreateUniqueID());
                        msg      += rst.first + "\n";
                    }
                }
                if (hud & (1 << HudHelper::HudType::hopper)) {
                    int ch = HopperCounterManager::getViewChannel(blockSource, hitrst);
                    if (ch != -1) msg += mod.getHopperCounterManager().getChannel(ch).info() + "\n";
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
    gt = (gt + 1) % 20;
}

} // namespace coral_fans::functions