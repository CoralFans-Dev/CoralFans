#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/BlockTickingQueue.h"


#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region, until, max, instaTick_);
#endif
    return origin(region, until, functions::MaxPtManager::getInstance().maxpt, instaTick_);
}

void MaxPtManager::hook(bool enabled) { enabled ? MaxPtHook::hook() : MaxPtHook::unhook(); }
} // namespace coral_fans::functions