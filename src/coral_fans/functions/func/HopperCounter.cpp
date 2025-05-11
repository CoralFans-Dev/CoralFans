#include "FuncManager.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/HopperBlockActor.h"

#include <format>
#include <string>

namespace coral_fans::functions {

void HopperCounterChannel::reset() {
    this->gameTick = 0;
    this->counterList.clear();
    this->gtCounter.clear();
}

std::string HopperCounterChannel::info() {
    using ll::i18n_literals::operator""_tr;
    unsigned long long n = 0;
    for (const auto& i : this->counterList) n += i.second;
    if (this->gameTick == 0 || n == 0) return "translate.hoppercounter.info.nodata"_tr();
    unsigned long long gtTotal = 0;
    for (const auto& i : this->gtCounter) gtTotal += i.second;
    auto retstr = "translate.hoppercounter.info.introduction"_tr(
        this->channel,
        n,
        static_cast<float>(n) * 1.0f / static_cast<float>(this->gameTick) * 72000,
        this->gameTick,
        static_cast<float>(this->gameTick) / 72000.0f,
        gtTotal,
        static_cast<float>(gtTotal) * 1.0f / static_cast<float>(1200) * 72000
    );
    for (const auto& i : this->counterList)
        retstr += std::format(
            "\n §7-§r {}: §l{}§r (§l{}§r/h)",
            i.first,
            i.second,
            static_cast<float>(i.second) * 1.0f / static_cast<float>(this->gameTick) * 72000
        );
    return retstr;
}

void HopperCounterChannel::add(std::string name, unsigned long long count) {
    this->counterList[name]         += count;
    this->gtCounter[this->gameTick] += count;
    // refresh gtCounter
    while (!this->gtCounter.empty()) {
        auto it = this->gtCounter.begin();
        if (this->gameTick - it->first >= 1200) {
            this->gtCounter.erase(it);
        } else {
            break;
        }
    }
}

const std::unordered_map<std::string, int> HopperCounterManager::HOPPER_COUNTER_MAP = {
    {"white_concrete",      0 },
    {"orange_concrete",     1 },
    {"magenta_concrete",    2 },
    {"light_blue_concrete", 3 },
    {"yellow_concrete",     4 },
    {"lime_concrete",       5 },
    {"pink_concrete",       6 },
    {"gray_concrete",       7 },
    {"light_gray_concrete", 8 },
    {"cyan_concrete",       9 },
    {"purple_concrete",     10},
    {"blue_concrete",       11},
    {"brown_concrete",      12},
    {"green_concrete",      13},
    {"red_concrete",        14},
    {"black_concrete",      15}
};

void HopperCounterManager::tick() {
    if (coral_fans::mod().getConfigDb()->get("functions.global.hoppercounter") == "true")
        for (auto& channel : this->channels) channel.tick();
}

int HopperCounterManager::getViewChannel(BlockSource& blockSource, HitResult hitrst) {
    if (!hitrst) return -1;
    const auto&                                          dest = blockSource.getBlock(hitrst.mBlock);
    std::unordered_map<std::string, int>::const_iterator it;
    if (utils::removeMinecraftPrefix(dest.getTypeName()) == "hopper") {
        int      var            = dest.mLegacyBlock->getVariant(dest);
        BlockPos pos            = hitrst.mBlock;
        pos[(var / 2 + 1) % 3] += (var & 1) * 2 - 1;
        const auto& block       = blockSource.getBlock(pos);
        it =
            functions::HopperCounterManager::HOPPER_COUNTER_MAP.find(utils::removeMinecraftPrefix(block.getTypeName()));
    } else
        it = functions::HopperCounterManager::HOPPER_COUNTER_MAP.find(utils::removeMinecraftPrefix(dest.getTypeName()));
    if (it != functions::HopperCounterManager::HOPPER_COUNTER_MAP.end()) return it->second;
    else return -1;
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansFunctionsHopperCounterHook1,
    ll::memory::HookPriority::Normal,
    HopperBlockActor,
    &HopperBlockActor::_tryMoveItems,
    bool,
    ::BlockSource& region,
    ::Container&   fromContainer,
    ::Vec3 const&  pos,
    int            attachedFace,
    bool           canPushItems
) {
    HopperCounterManager::getInstance().region = &region;
    HopperCounterManager::getInstance().pos    = pos;
    HopperCounterManager::getInstance().mutex  = true;
    bool ori                                   = origin(region, fromContainer, pos, attachedFace, canPushItems);
    HopperCounterManager::getInstance().mutex  = false;
    return ori;
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansFunctionsHopperCounterHook2,
    ll::memory::HookPriority::Normal,
    HopperBlockActor,
    &HopperBlockActor::$setItem,
    void,
    int                slot,
    ::ItemStack const& item
) {
    if (!HopperCounterManager::getInstance().mutex) {
        HOOK_HOPPER_RETURN
    }
    // get dest block
    // auto& blockActor = ll::memory::dAccess<BlockActor>(this, -200); // magic number!
    BlockPos     pos        = HopperCounterManager::getInstance().pos;
    const Block& block      = HopperCounterManager::getInstance().region->getBlock(pos);
    int          var        = block.mLegacyBlock->getVariant(block);
    pos[(var / 2 + 1) % 3] += (var & 1) * 2 - 1;
    auto& dest              = HopperCounterManager::getInstance().region->getBlock(pos);
    // get iterator
    auto it = HopperCounterManager::HOPPER_COUNTER_MAP.find(utils::removeMinecraftPrefix(dest.getTypeName()));
    if (it == HopperCounterManager::HOPPER_COUNTER_MAP.end()) {
        HOOK_HOPPER_RETURN
    }
    // get channel
    auto channel = it->second;
    if (channel < 0 || channel > 15 || item.getName().empty()) {
        HOOK_HOPPER_RETURN
    }
    // save item info
    coral_fans::mod().getHopperCounterManager().getChannel(channel).add(
        item.getCustomName().empty() ? item.getName() : item.getCustomName() + " (" + item.getTypeName() + ")",
        item.mCount
    );
    // remove item
    origin(slot, ItemStack::EMPTY_ITEM());
}

void hookFunctionsHopperCounter(bool hook) {
    if (hook) {
        CoralFansFunctionsHopperCounterHook1::hook();
        CoralFansFunctionsHopperCounterHook2::hook();
    } else {
        CoralFansFunctionsHopperCounterHook1::unhook();
        CoralFansFunctionsHopperCounterHook2::unhook();
    }
}

} // namespace coral_fans::functions