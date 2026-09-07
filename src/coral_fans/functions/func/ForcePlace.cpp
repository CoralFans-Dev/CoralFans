#include "ll/api/memory/Hook.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"


#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


namespace coral_fans::functions {
// force place
LL_TYPE_INSTANCE_HOOK(
    CoralFansForcePlaceHook1,
    ll::memory::HookPriority::Normal,
    BlockSource,
    &BlockSource::$mayPlace,
    bool,
    Block const&    block,
    BlockPos const& pos,
    uchar           face,
    Actor*          placer,
    bool,
    Vec3 clickPos
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(block, pos, face, placer, true, clickPos);
#endif
    return origin(block, pos, face, placer, true, clickPos);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansForcePlaceHook2,
    ll::memory::HookPriority::Normal,
    BlockSource,
    &BlockSource::$mayPlace,
    bool,
    Block const&    block,
    BlockPos const& pos,
    uchar           face,
    Actor*          placer,
    bool            val,
    Vec3            clickPos
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(block, pos, face, placer, val, clickPos);
#endif
    if (getBlock(pos).isAir()) return true;
    auto stone = Block::tryGetFromRegistry("minecraft:stone");
    return origin(stone, pos, face, placer, val, clickPos);
}

void forcePlaceHook(uint64 type) {
    switch (type) {
    case 0:
        CoralFansForcePlaceHook1::unhook();
        CoralFansForcePlaceHook2::unhook();
        break;
    case 1:
        CoralFansForcePlaceHook1::hook();
        CoralFansForcePlaceHook2::unhook();
    case 2:
        CoralFansForcePlaceHook1::unhook();
        CoralFansForcePlaceHook2::hook();
    }
}
} // namespace coral_fans::functions