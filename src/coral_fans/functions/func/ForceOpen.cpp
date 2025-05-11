#include "ll/api/memory/Hook.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"


namespace coral_fans::functions {
// force open
LL_TYPE_INSTANCE_HOOK(CoralFansForceOpenHook, ll::memory::HookPriority::Normal, ChestBlockActor, &ChestBlockActor::$_canOpenThis, bool, BlockSource&) {
    return true;
}

void forceOpenHook(bool bl) { bl ? CoralFansForceOpenHook::hook() : CoralFansForceOpenHook ::unhook(); }
} // namespace coral_fans::functions