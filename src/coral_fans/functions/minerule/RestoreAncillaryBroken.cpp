#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockType.h"
#include <cstddef>
#include <thread>
#include <vector>


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    RestoreAncillaryBrokenHook1,
    ll::memory::HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_spawnMovingBlocks,
    void,
    ::BlockSource& region
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region);
#endif
    auto& helper            = RestoreAncillaryBrokenHelper::getInstance();
    helper.mutex            = true;
    helper.mutex2           = true;
    helper.pistonBlockActor = this;
    origin(region);
    helper.mutex  = false;
    helper.mutex2 = false;
}


LL_TYPE_INSTANCE_HOOK(
    RestoreAncillaryBrokenHook2,
    HookPriority::Normal,
    BlockType,
    &BlockType ::spawnResources,
    void,
    ::BlockSource&                region,
    ::BlockPos const&             pos,
    ::Block const&                block,
    ::IRandom&                    random,
    ::ResourceDropsContext const& resourceDropsContext
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region, pos, block, random, resourceDropsContext);
#endif
    auto& helper = RestoreAncillaryBrokenHelper::getInstance();
    if (helper.mutex2) {
        helper.mutex2       = false;
        helper._randomize   = &random;
        helper.dropsContext = &resourceDropsContext;
        origin(region, pos, block, random, resourceDropsContext);
        return;
    }
    origin(region, pos, block, random, resourceDropsContext);
}

LL_TYPE_INSTANCE_HOOK(
    RestoreAncillaryBrokenHook3,
    ll::memory::HookPriority::Normal,
    Level,
    &Level::$destroyBlock,
    bool,
    ::BlockSource&              region,
    ::BlockPos const&           pos,
    bool                        dropResources,
    const ::BlockChangeContext& blockChangeContext
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region, pos, dropResources, blockChangeContext);
#endif
    auto& helper = RestoreAncillaryBrokenHelper::getInstance();
    if (helper.mutex) {
        helper.mutex2  = true;
        auto&    block = region.getBlock(pos);
        BlockPos secondPartPos;
        bool     hasSecondPart = block.mBlockType->getSecondPart(region, pos, secondPartPos);
        auto     ori           = origin(region, pos, dropResources, blockChangeContext);
        if (hasSecondPart) {
            auto& secondPartBlock = region.getBlock(secondPartPos);
            if (block.mBlockType != secondPartBlock.mBlockType) {
                secondPartBlock.mBlockType
                    ->spawnResources(region, secondPartPos, secondPartBlock, *helper._randomize, *helper.dropsContext);
                auto& secondPartLiquidBlock = region.getLiquidBlock(secondPartPos);
                secondPartLiquidBlock.mBlockType->spawnResources(
                    region,
                    secondPartPos,
                    secondPartLiquidBlock,
                    *helper._randomize,
                    *helper.dropsContext
                );
                region.removeBlock(secondPartPos, blockChangeContext);
                size_t length = helper.pistonBlockActor->mBreakBlocks->size();
                for (size_t i = 0; i < length; i++) {
                    if ((*helper.pistonBlockActor->mBreakBlocks)[i] == secondPartPos) {
                        helper.pistonBlockActor->mBreakBlocks->erase(
                            helper.pistonBlockActor->mBreakBlocks->begin() + i
                        );
                    }
                }
            }
        }
        return ori;
    }
    return origin(region, pos, dropResources, blockChangeContext);
}

void restoreAncillaryBrokenHook(bool bl) {
    if (bl) {
        RestoreAncillaryBrokenHook1::hook();
        RestoreAncillaryBrokenHook2::hook();
        RestoreAncillaryBrokenHook3::hook();
    } else {
        RestoreAncillaryBrokenHook1::unhook();
        RestoreAncillaryBrokenHook2::unhook();
        RestoreAncillaryBrokenHook3::unhook();
    }
}
} // namespace coral_fans::functions