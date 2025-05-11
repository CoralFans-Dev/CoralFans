#include "ll/api/memory/Hook.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockSource.h"


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
    return origin(block, pos, face, placer, true, clickPos);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansForcePlaceHook2,
    ll::memory::HookPriority::Normal,
    BlockSource,
    &BlockSource::$mayPlace,
    bool,
    Block const&,
    BlockPos const&,
    uchar,
    Actor*,
    bool,
    Vec3
) {
    return true;
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