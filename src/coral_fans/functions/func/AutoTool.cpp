#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Utils.h"


#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/memory/Hook.h"
#include "mc/entity/components/PlayerBlockActionData.h"
#include "mc/entity/components/PlayerBlockActions.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/ServerPlayerBlockUseHandler.h"
#include "mc/network/packet/PlayerActionType.h"
#include "mc/network/packet/PlayerAuthInputPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/gamemode/SurvivalMode.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include <optional>


#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


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
LL_TYPE_INSTANCE_HOOK(
    CoralFansAutoToolHook1,
    ll::memory::HookPriority::Normal,
    SurvivalMode,
    &SurvivalMode::$continueDestroyBlock,
    bool,
    ::BlockPos const& pos,
    uchar             face,
    ::Vec3 const&     playerPos,
    bool&             hasDestroyedBlock
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(pos, face, playerPos, hasDestroyedBlock);
#endif
    // CoralFans::getInstance().getSelf().getLogger().info(
    //     "before continueDestroyBlock called. pos: {} isAir: {}\nmOldDestroyProgress: {}\nmDestroyProgress: {}",
    //     pos,
    //     mPlayer.getDimensionBlockSource().getBlock(pos).isAir(),
    //     mOldDestroyProgress,
    //     mDestroyProgress
    // );
    // mDestroyBlockPos = pos;
    // auto ori         = origin(pos, face, playerPos, hasDestroyedBlock);
    // CoralFans::getInstance().getSelf().getLogger().info(
    //     "after continueDestroyBlock called. result: {}\nmOldDestroyProgress: {}\nmDestroyProgress: {}",
    //     ori,
    //     mOldDestroyProgress,
    //     mDestroyProgress
    // );
    // return ori;

    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.autotool", mPlayer.getUuid().asString())
        )
        == "true") {
        int currentSlot = mPlayer.getSelectedItemSlot();
        int minDamage   = std::stoi(
            CoralFans::getInstance()
                .getConfigDb()
                ->get(std::format("functions.players.{}.autotool.mindamage", mPlayer.getUuid().asString()))
                .value_or("1")
        );
        const Block& block = mPlayer.getDimensionBlockSource().getBlock(pos);
        int bestSlot = ::searchBestToolInInv(*mPlayer.mInventory->mInventory, currentSlot, &block, minDamage, false);
        if (bestSlot > 8) {
            utils::sendInventorySwap(&mPlayer, currentSlot, bestSlot);
            mPlayer.refreshInventory();
        } else if (bestSlot >= 0) {
            mPlayer.setSelectedSlot(bestSlot);
        }
    }
    return origin(pos, face, playerPos, hasDestroyedBlock);
}

void handleAutoWeapon(Player& player) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return;
#endif
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
