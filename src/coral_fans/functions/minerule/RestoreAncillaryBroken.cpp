#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockType.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include <cstddef>
#include <vector>


namespace coral_fans::functions {
bool                        mutex  = false;
bool                        mutex2 = false;
const ResourceDropsContext* dropsContext;
Randomize*                  _randomize;
PistonBlockActor*           pistonBlockActor;

LL_TYPE_INSTANCE_HOOK(
    CoralFansRestoreAncillaryBrokenHook1,
    ll::memory::HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_spawnMovingBlocks,
    void,
    ::BlockSource& region
) {
    mutex            = true;
    mutex2           = true;
    pistonBlockActor = this;
    origin(region);
    mutex  = false;
    mutex2 = false;
}


LL_TYPE_INSTANCE_HOOK(
    CoralFansRestoreAncillaryBrokenHook2,
    HookPriority::Normal,
    BlockType,
    &BlockType ::spawnResources,
    void,
    ::BlockSource&                region,
    ::BlockPos const&             pos,
    ::Block const&                block,
    ::Randomize&                  randomize,
    ::ResourceDropsContext const& resourceDropsContext
) {
    if (mutex2) {
        mutex2       = false;
        _randomize   = &randomize;
        dropsContext = &resourceDropsContext;
        origin(region, pos, block, randomize, resourceDropsContext);
        return;
    }
    origin(region, pos, block, randomize, resourceDropsContext);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansRestoreAncillaryBrokenHook3,
    ll::memory::HookPriority::Normal,
    Level,
    &Level::$destroyBlock,
    bool,
    ::BlockSource&              region,
    ::BlockPos const&           pos,
    bool                        dropResources,
    const ::BlockChangeContext& blockChangeContext
) {
    if (mutex) {
        mutex2         = true;
        auto&    block = region.getBlock(pos);
        BlockPos secondPartPos;
        bool     hasSecondPart = block.mBlockType->getSecondPart(region, pos, secondPartPos);
        auto     ori           = origin(region, pos, dropResources, blockChangeContext);
        if (hasSecondPart) {
            auto& secondPartBlock = region.getBlock(secondPartPos);
            if (block.mBlockType != secondPartBlock.mBlockType) {
                secondPartBlock.mBlockType
                    ->spawnResources(region, secondPartPos, secondPartBlock, *_randomize, *dropsContext);
                auto& secondPartLiquidBlock = region.getLiquidBlock(secondPartPos);
                secondPartLiquidBlock.mBlockType
                    ->spawnResources(region, secondPartPos, secondPartLiquidBlock, *_randomize, *dropsContext);
                region.removeBlock(secondPartPos, blockChangeContext);
                size_t length = pistonBlockActor->mBreakBlocks->size();
                for (size_t i = 0; i < length; i++) {
                    if ((*pistonBlockActor->mBreakBlocks)[i] == secondPartPos) {
                        pistonBlockActor->mBreakBlocks->erase(pistonBlockActor->mBreakBlocks->begin() + i);
                    }
                }
            }
        }
        return ori;
    }
    return origin(region, pos, dropResources, blockChangeContext);
}

void MineruleManager::restoreAncillaryBrokenHook(bool bl) {
    if (bl) {
        CoralFansRestoreAncillaryBrokenHook1::hook();
        CoralFansRestoreAncillaryBrokenHook2::hook();
        CoralFansRestoreAncillaryBrokenHook3::hook();
    } else {
        CoralFansRestoreAncillaryBrokenHook1::unhook();
        CoralFansRestoreAncillaryBrokenHook2::unhook();
        CoralFansRestoreAncillaryBrokenHook3::unhook();
    }
}
} // namespace coral_fans::functions