#include "coral_fans/CoralFans.h"


#include "coral_fans/base/Macros.h"
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
    RETURN_IF_NOT_MAIN_THREAD(return origin(itemActor, orgCount, favoredSlot));
    if (itemActor.hasCategory(ActorCategory::Item)
        && CoralFans::getInstance().getConfigDb()->get(
               std::format("functions.players.{}.nopickup", this->getUuid().asString())
           ) == "true")
        return false;
    return origin(itemActor, orgCount, favoredSlot);
}

void noPickUpHook(bool bl) { bl ? CoralFansNoPickUpHook ::hook() : CoralFansNoPickUpHook ::unhook(); }
} // namespace coral_fans::functions