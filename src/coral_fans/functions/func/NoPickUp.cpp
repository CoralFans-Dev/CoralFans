#include "coral_fans/CoralFans.h"


#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Player.h"


#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(itemActor, orgCount, favoredSlot);
#endif
    if (itemActor.hasCategory(ActorCategory::Item)
        && CoralFans::getInstance().getConfigDb()->get(
               std::format("functions.players.{}.nopickup", this->getUuid().asString())
           ) == "true")
        return false;
    return origin(itemActor, orgCount, favoredSlot);
}

void noPickUpHook(bool bl) { bl ? CoralFansNoPickUpHook ::hook() : CoralFansNoPickUpHook ::unhook(); }
} // namespace coral_fans::functions