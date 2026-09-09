#include "MineruleManager.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/BlockTickingQueue.h"


namespace coral_fans::functions {

// BlockTickingQueue tickPendingTicks
LL_TYPE_INSTANCE_HOOK(
    MaxPtHook,
    ll::memory::HookPriority::Lowest,
    BlockTickingQueue,
    &BlockTickingQueue::tickPendingTicks,
    bool,
    BlockSource& region,
    Tick const&  until,
    int          max,
    bool         instaTick_
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(region, until, max, instaTick_));
    return origin(region, until, functions::MaxPtManager::getInstance().maxpt, instaTick_);
}

void MaxPtManager::hook(bool enabled) { enabled ? MaxPtHook::hook() : MaxPtHook::unhook(); }
} // namespace coral_fans::functions