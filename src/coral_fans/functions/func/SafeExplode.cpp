#include "ll/api/memory/Hook.h"
#include "mc/world/level/Explosion.h"


#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif

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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(random);
#endif
    return false;
}

void safeExplodeHook(bool bl) { bl ? CoralFansSafeExplodeHook::hook() : CoralFansSafeExplodeHook::unhook(); }
} // namespace coral_fans::functions