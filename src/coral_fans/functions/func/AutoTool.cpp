#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"


#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/memory/Hook.h"
#include "mc/network/ServerPlayerBlockUseHandler.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"


#include <format>
#include <string>

namespace {

ll::event::ListenerPtr gAutoWeaponListener;

struct ToolInfo {
    float value        = 0;
    int   slot         = -1;
    short remainDamage = 0;
};

int searchBestToolInInv(Container& inv, int currentSlot, const Block* block, const int minDamage, bool weapon) {
    if (!weapon && !block) return currentSlot;
    auto&    currentItem = inv.getItem(currentSlot);
    ToolInfo curInfo;
    if (currentItem != ItemStack::EMPTY_ITEM()) {
        short currentRemainDamage = currentItem.mItem->getMaxDamage() - currentItem.getDamageValue();
        if (currentRemainDamage > minDamage) {
            curInfo = {
                weapon ? currentItem.mItem.get()->getAttackDamage()
                       : currentItem.mItem.get()->getDestroySpeed(currentItem, *block),
                currentSlot,
                currentRemainDamage
            };
        }
    }
    int size = inv.getContainerSize();
    for (int i = 0; i < size; ++i) {
        auto& item = inv.getItem(i);
        if (item != ItemStack::EMPTY_ITEM()) {
            float value =
                weapon ? item.mItem.get()->getAttackDamage() : item.mItem.get()->getDestroySpeed(currentItem, *block);
            short remainDamage = item.mItem->getMaxDamage() - item.getDamageValue();
            // skip low remainDamage tools
            if (remainDamage <= minDamage) continue;
            if (value > curInfo.value) {
                curInfo = {value, i, remainDamage};
            }
        }
    }
    return curInfo.slot;
}

} // namespace

namespace coral_fans::functions {

LL_STATIC_HOOK(
    CoralFansAutoToolHook1,
    ll::memory::HookPriority::Normal,
    &ServerPlayerBlockUseHandler::onStartDestroyBlock,
    void,
    ServerPlayer&   player,
    const BlockPos& pos,
    int             face
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(player, pos, face));
    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.autotool", player.getUuid().asString())
        )
        == "true") {
        int currentSlot = player.getSelectedItemSlot();
        int minDamage   = std::stoi(
            CoralFans::getInstance()
                .getConfigDb()
                ->get(std::format("functions.players.{}.autotool.mindamage", player.getUuid().asString()))
                .value_or("1")
        );
        const Block& block = player.getDimensionBlockSource().getBlock(pos);
        int bestSlot = ::searchBestToolInInv(*player.mInventory->mInventory, currentSlot, &block, minDamage, false);
        if (bestSlot > 8) {
            utils::sendInventorySwap(&player, currentSlot, bestSlot);
            player.refreshInventory();
        } else if (bestSlot >= 0) {
            player.setSelectedSlot(bestSlot);
        }
    }
    return origin(player, pos, face);
}

void handleAutoWeapon(Player& player) {
    RETURN_VOID_IF_NOT_MAIN_THREAD;
    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.autoweapon", player.getUuid().asString())
        )
        == "true") {
        int currentSlot = player.getSelectedItemSlot();
        int minDamage   = std::stoi(
            CoralFans::getInstance()
                .getConfigDb()
                ->get(std::format("functions.players.{}.autoweapon.mindamage", player.getUuid().asString()))
                .value_or("1")
        );
        int bestSlot = ::searchBestToolInInv(*player.mInventory->mInventory, currentSlot, nullptr, minDamage, true);
        if (bestSlot > 8) {
            utils::sendInventorySwap(&player, currentSlot, bestSlot);
            player.refreshInventory();
        } else if (bestSlot >= 0) {
            player.setSelectedSlot(bestSlot);
        }
    }
}

void hookAutoTool(bool hook) {
    if (hook) {
        CoralFansAutoToolHook1::hook();
    } else {
        CoralFansAutoToolHook1::unhook();
    }
}

void hookAutoWeapon(bool hook) {
    auto& bus = ll::event::EventBus::getInstance();
    if (hook) {
        if (gAutoWeaponListener) return;
        gAutoWeaponListener =
            bus.emplaceListener<ll::event::player::PlayerAttackEvent>([](ll::event::player::PlayerAttackEvent& event) {
                handleAutoWeapon(event.self());
            });
    } else {
        if (gAutoWeaponListener) {
            bus.removeListener<ll::event::player::PlayerAttackEvent>(gAutoWeaponListener);
            gAutoWeaponListener = nullptr;
        }
    }
}

} // namespace coral_fans::functions
