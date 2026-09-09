#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"


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
    RETURN_IF_NOT_MAIN_THREAD(return origin(block, pos, face, placer, true, clickPos));
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
    RETURN_IF_NOT_MAIN_THREAD(return origin(block, pos, face, placer, val, clickPos));
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