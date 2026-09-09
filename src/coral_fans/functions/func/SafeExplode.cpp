#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/Explosion.h"


namespace coral_fans::functions {
// safeexplode
LL_TYPE_INSTANCE_HOOK(
    CoralFansSafeExplodeHook,
    ll::memory::HookPriority::Normal,
    Explosion,
    &Explosion::explode,
    bool,
    ::IRandom& random
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(random));
    return false;
}

void safeExplodeHook(bool bl) { bl ? CoralFansSafeExplodeHook::hook() : CoralFansSafeExplodeHook::unhook(); }
} // namespace coral_fans::functions