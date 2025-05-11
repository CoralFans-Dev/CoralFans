#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Player.h"

namespace coral_fans::functions {
// nopickup
LL_TYPE_INSTANCE_HOOK(
    CoralFansNoPickUpHook,
    HookPriority::Normal,
    Player,
    &Player::take,
    bool,
    Actor& itemActor,
    int    orgCount,
    int    favoredSlot
) {
    if (itemActor.hasCategory(ActorCategory::Item)
        && coral_fans::mod().getConfigDb()->get(std::format("functions.players.{}.nopickup", this->getUuid().asString())
           ) == "true")
        return false;
    return origin(itemActor, orgCount, favoredSlot);
}

void noPickUpHook(bool bl) { bl ? CoralFansNoPickUpHook ::hook() : CoralFansNoPickUpHook ::unhook(); }
} // namespace coral_fans::functions