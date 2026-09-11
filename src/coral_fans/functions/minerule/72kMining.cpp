#include "coral_fans/CoralFans.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/SurvivalMode.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"

#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    Mining72kToolHook1,
    ll::memory::HookPriority::Normal,
    SurvivalMode,
    &SurvivalMode::$continueDestroyBlock,
    bool,
    ::BlockPos const& pos,
    uchar             face,
    ::Vec3 const&     playerPos,
    bool&             hasDestroyedBlock
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(pos, face, playerPos, hasDestroyedBlock);
#endif
    if (!mOldDestroyProgress) mDestroyBlockPos = pos;
    return origin(pos, face, playerPos, hasDestroyedBlock);
    // 服务端客户端同时激活时可以跳过挖掘冷却
    // if (ori) mNoDestroyUntil = mTimer->getCurrentDestroyDelayTime();
}

void miningHook(bool enable) { enable ? Mining72kToolHook1 ::hook() : Mining72kToolHook1 ::unhook(); }
} // namespace coral_fans::functions