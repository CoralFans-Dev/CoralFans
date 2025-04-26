#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Player.h"


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    CoralFansPortalDisabled,
    ll::memory::HookPriority::Normal,
    Player,
    &Player::$canChangeDimensionsUsingPortal,
    bool
) {
    if (coral_fans::mod().getConfigDb()->get(
            std::format("functions.players.{}.portaldisabled", this->getUuid().asString())
        )
        == "true")
        return false;
    else return origin();
}

void portalDisabledHook(bool bl) { bl ? CoralFansPortalDisabled::hook() : CoralFansPortalDisabled::unhook(); }
} // namespace coral_fans::functions