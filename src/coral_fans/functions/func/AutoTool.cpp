#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/events/BlockEventCoordinator.h"
#include "mc/world/item/ItemStack.h"


#include <format>
#include <string>

namespace {

struct ToolInfo {
    float value;
    int   slot;
    short remainDamage;
};

int searchBestToolInInv(Container& inv, int currentSlot, const Block* block, const int minDamage, bool weapon) {
    if (!weapon && !block) return currentSlot;
    auto&    currentItem = inv.getItem(currentSlot);
    ToolInfo curInfo{
        weapon ? currentItem.getAttackDamage() : currentItem.getDestroySpeed(*block),
        currentSlot,
        currentItem.getMaxDamage() - currentItem.getDamageValue()
    };
    int size = inv.getContainerSize();
    for (int i = 0; i < size; ++i) {
        auto& item = inv.getItem(i);
        if (item.mCount != 0) {
            float value        = weapon ? item.getAttackDamage() : item.getDestroySpeed(*block);
            short remainDamage = item.getMaxDamage() - item.getDamageValue();
            // skip low remainDamage tools
            if (remainDamage <= minDamage) continue;
            if (value >= curInfo.value) {
                curInfo = {value, i, remainDamage};
            }
        }
    }
    return curInfo.slot;
}

} // namespace

namespace coral_fans::functions {

LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersAutoToolHook1,
    ll::memory::HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendBlockDestructionStarted,
    void,
    Player&         player,
    BlockPos const& blockPos,
    Block const&    block,
    uchar           unk_char
) {
    if (coral_fans::mod().getConfigDb()->get("functions.global.autotool") == "true"
        && coral_fans::mod().getConfigDb()->get(
               std::format("functions.players.{}.autotool", player.getUuid().asString())
           ) == "true") {
        int currentSlot = player.getSelectedItemSlot();
        int minDamage =
            std::stoi(coral_fans::mod()
                          .getConfigDb()
                          ->get(std::format("functions.players.{}.autotool.mindamage", player.getUuid().asString()))
                          .value_or("1"));
        int bestSlot = ::searchBestToolInInv(player.getInventory(), currentSlot, &block, minDamage, false);
        if (bestSlot <= 8) {
            player.setSelectedSlot(bestSlot);
        } else {
            utils::swapItemInContainer(&player, currentSlot, bestSlot);
            player.refreshInventory();
        }
    }
    return origin(player, blockPos, block, unk_char);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersAutoToolHook2,
    ll::memory::HookPriority::Normal,
    Player,
    &Player::$attack,
    bool,
    Actor&                    actor,
    const ::ActorDamageCause& cause
) {
    if (coral_fans::mod().getConfigDb()->get("functions.global.autotool") == "true"
        && coral_fans::mod().getConfigDb()->get(std::format("functions.players.{}.autotool", this->getUuid().asString())
           ) == "true") {
        int currentSlot = this->getSelectedItemSlot();
        int minDamage =
            std::stoi(coral_fans::mod()
                          .getConfigDb()
                          ->get(std::format("functions.players.{}.autotool.mindamage", this->getUuid().asString()))
                          .value_or("1"));
        int bestSlot = ::searchBestToolInInv(this->getInventory(), currentSlot, nullptr, minDamage, true);
        if (bestSlot <= 8) {
            this->setSelectedSlot(bestSlot);
        } else {
            utils::swapItemInContainer(this, currentSlot, bestSlot);
            this->refreshInventory();
        }
    }
    return origin(actor, cause);
}

void hookTweakersAutoTool(bool hook) {
    if (hook) {
        CoralFansTweakersAutoToolHook1::hook();
        CoralFansTweakersAutoToolHook2::hook();
    } else {
        CoralFansTweakersAutoToolHook1::unhook();
        CoralFansTweakersAutoToolHook2::unhook();
    }
}

} // namespace coral_fans::functions
