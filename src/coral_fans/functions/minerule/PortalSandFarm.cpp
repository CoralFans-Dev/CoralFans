#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include "mc/world/actor/item/FallingBlockActor.h"
#include <thread>

namespace coral_fans::functions {

LL_TYPE_INSTANCE_HOOK(
    CoralFansportalSandFarmHook,
    ll::memory::HookPriority::Normal,
    FallingBlockActor,
    &FallingBlockActor::$teleportTo,
    void,
    ::Vec3 const& pos,
    bool          shouldStopRiding,
    int           cause,
    int           sourceEntityType,
    bool          keepVelocity
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(pos, shouldStopRiding, cause, sourceEntityType, keepVelocity);
#endif
    this->mState = State::Falling;
    return origin(pos, shouldStopRiding, cause, sourceEntityType, keepVelocity);
}

void portalSandFarmHook(bool bl) { bl ? CoralFansportalSandFarmHook::hook() : CoralFansportalSandFarmHook::unhook(); }

} // namespace coral_fans::functions