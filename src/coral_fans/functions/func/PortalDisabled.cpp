#include "coral_fans/CoralFans.h"


#include "ll/api/memory/Hook.h"
#include "mc/server/ServerPlayer.h"


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    CoralFansPortalDisabled,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$canChangeDimensionsUsingPortal,
    bool
) {
    if (CoralFans::getInstance().getConfigDb()->get(
            std::format("functions.players.{}.portaldisabled", this->getUuid().asString())
        )
        == "true")
        return false;
    else return origin();
}

void portalDisabledHook(bool bl) { bl ? CoralFansPortalDisabled::hook() : CoralFansPortalDisabled::unhook(); }
} // namespace coral_fans::functions