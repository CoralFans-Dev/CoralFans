#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/BlockEventCoordinator.h"

#include <format>
#include <string>

namespace {

struct ToolInfo {
    float speed;
    int   slot;
    short remainDamage;
};

int searchBestToolInInv(Container& inv, int currentSlot, const Block& block, const int minDamage) {
    auto&    currentItem = inv.getItem(currentSlot);
    ToolInfo curInfo{
        currentItem.getDestroySpeed(block),
        currentSlot,
        currentItem.getMaxDamage() - currentItem.getDamageValue()
    };
    int size = inv.getContainerSize();
    for (int i = 0; i < size; ++i) {
        auto& item = inv.getItem(i);
        if (item.mCount != 0) {
            float speed        = item.getDestroySpeed(block);
            short remainDamage = item.getMaxDamage() - item.getDamageValue();
            // skip low remainDamage tools
            if (remainDamage <= minDamage) continue;
            if (speed >= curInfo.speed) {
                curInfo = {speed, i, remainDamage};
            }
        }
    }
    return curInfo.slot;
}

void swapItemInContainer(Container& container, int slot1, int slot2) {
    auto i1 = container.getItem(slot1).clone();
    auto i2 = container.getItem(slot2).clone();
    container.removeItem(slot1, 64);
    container.removeItem(slot2, 64);
    container.setItem(slot1, i2);
    container.setItem(slot2, i1);
}

} // namespace

namespace coral_fans::functions {

LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersAutoToolHook,
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
        int bestSlot = ::searchBestToolInInv(player.getInventory(), currentSlot, block, minDamage);
        if (bestSlot <= 8) {
            player.setSelectedSlot(bestSlot);
        } else {
            ::swapItemInContainer(player.getInventory(), currentSlot, bestSlot);
        }
    }
    origin(player, blockPos, block, unk_char);
}

} // namespace coral_fans::functions
