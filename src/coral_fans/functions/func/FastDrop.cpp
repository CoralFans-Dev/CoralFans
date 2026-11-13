#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/helper/MainThreadExecutor.h"

#include "ll/api/coro/CoroTask.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/level/Level.h"


namespace coral_fans::functions {

// fastdrop
LL_TYPE_INSTANCE_HOOK(
    CoralFansFastDropHook,
    ll::memory::HookPriority::Normal,
    Player,
    &Player::$drop,
    bool,
    ItemStack const& item,
    bool             randomly
) {
    RETURN_IF_NOT_MAIN_THREAD(origin(item, randomly));
    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.fastdrop", this->getUuid().asString())
        )
        != "true")
        return origin(item, randomly);

    bool result = origin(item, randomly);
    if (!result) return false;

    ll::coro::keepThis([uuid = this->getUuid(), itemType = ItemStack(item), randomly]() -> ll::coro::CoroTask<> {
        auto level = ll::service::getLevel();
        if (!level.has_value()) co_return;
        Player* player = level->getPlayer(uuid);
        if (!player) co_return;
        if (CoralFans::getInstance().getConfigDb()->get(std::format("functions.players.{}.fastdrop", uuid.asString()))
            != "true")
            co_return;

        auto& inv     = *player->mInventory->mInventory;
        int   size    = inv.getContainerSize();
        bool  changed = false;
        for (int i = 0; i < size; ++i) {
            const auto& itemi = inv.getItem(i);
            if (!itemi.isNull() && itemi.matchesItem(itemType))
                if (player->drop(itemi, randomly)) {
                    inv.setItem(i, ItemStack::EMPTY_ITEM());
                    changed = true;
                }
        }
        if (changed) player->refreshInventory();
    }).launch(helper::thread::MainThreadExecutor::getDefault(), [](ll::Expected<>) {});
    return result;
}

void fastDropHook(bool bl) { bl ? CoralFansFastDropHook ::hook() : CoralFansFastDropHook ::unhook(); }
} // namespace coral_fans::functions
