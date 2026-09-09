#include "MineruleManager.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/item/FallingBlockActor.h"


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
    RETURN_IF_NOT_MAIN_THREAD(return origin(pos, shouldStopRiding, cause, sourceEntityType, keepVelocity));
    this->mState = State::Falling;
    return origin(pos, shouldStopRiding, cause, sourceEntityType, keepVelocity);
}

void portalSandFarmHook(bool bl) { bl ? CoralFansportalSandFarmHook::hook() : CoralFansportalSandFarmHook::unhook(); }

} // namespace coral_fans::functions