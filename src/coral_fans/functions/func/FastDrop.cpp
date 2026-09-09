#include "coral_fans/CoralFans.h"


#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"


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
    RETURN_IF_NOT_MAIN_THREAD(return origin(item, randomly));
    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.fastdrop", this->getUuid().asString())
        )
        == "true") {
        auto& inv  = *this->mInventory->mInventory;
        int   size = inv.getContainerSize();
        for (int i = 0; i < size; ++i) {
            const auto& itemi = inv.getItem(i);
            if (itemi.matchesItem(item))
                if (origin(itemi, randomly)) inv.setItem(i, ItemStack::EMPTY_ITEM());
        }
        this->refreshInventory();
        return false;
    } else return origin(item, randomly);
}

void fastDropHook(bool bl) { bl ? CoralFansFastDropHook ::hook() : CoralFansFastDropHook ::unhook(); }
} // namespace coral_fans::functions