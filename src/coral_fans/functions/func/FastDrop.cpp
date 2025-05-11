#include "coral_fans/base/Mod.h"
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
    if (coral_fans::mod().getConfigDb()->get(std::format("functions.players.{}.fastdrop", this->getUuid().asString()))
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