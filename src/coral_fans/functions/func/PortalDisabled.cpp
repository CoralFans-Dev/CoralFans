#include "coral_fans/CoralFans.h"


#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include "mc/server/ServerPlayer.h"
#include <thread>


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    CoralFansPortalDisabled,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$canChangeDimensionsUsingPortal,
    bool
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.portaldisabled", this->getUuid().asString())
        )
        == "true")
        return false;
    else return origin();
}

void portalDisabledHook(bool bl) { bl ? CoralFansPortalDisabled::hook() : CoralFansPortalDisabled::unhook(); }
} // namespace coral_fans::functions