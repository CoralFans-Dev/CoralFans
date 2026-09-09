#include "MineruleManager.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "mc/scripting/modules/minecraft/events/ScriptBlockGlobalEventListener.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockChangeContext.h"
#include "mc/world/level/block/BlockType.h"
#include "mc/world/level/block/ResourceDrops.h"
#include "mc/world/level/block/ResourceDropsContext.h"
#include "mc/world/level/block/actor/MovingBlockActor.h"
#include "mc/world/level/block/components/BlockComponentDirectData.h"
#include "mc/world/level/dimension/Dimension.h"
#include <mc/world/level/BlockSource.h>


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    CoralFansDropHook1,
    ll::memory::HookPriority::Normal,
    BlockType,
    &BlockType::getResourceDrops,
    ResourceDrops,
    ::Block const&                block,
    ::IRandom&                    random,
    ::ResourceDropsContext const& resourceDropsContext
) {
    RETURN_IF_NOT_MAIN_THREAD(
        return origin(block, random, std::forward<ResourceDropsContext const&>(resourceDropsContext))
    );
    if (block.getTypeName() == "minecraft:bedrock") {
        ItemStack itemStack;
        itemStack.reinit("bedrock", 1, 0);
        ResourceDrops res{};
        res.mItems = std::vector<ItemStack>{std::move(itemStack)};
        return res;
    }
    return origin(block, random, std::forward<ResourceDropsContext const&>(resourceDropsContext));
}

LL_TYPE_STATIC_HOOK(
    CoralFansDropHook2,
    HookPriority::Normal,
    Explosion,
    &Explosion ::_spawnExtraResourcesAndMergeItemDropsForBlock,
    void,
    ::BlockSource&                                       region,
    ::BlockPos const&                                    blockPos,
    ::Block const&                                       block,
    ::IRandom&                                           random,
    ::ResourceDropsContext const&                        resourceDropsContext,
    ::std::vector<::std::pair<::ItemStack, ::BlockPos>>& itemStacks
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(region, blockPos, block, random, resourceDropsContext, itemStacks));
    if (block.getTypeName() == "minecraft:moving_block") {
        MovingBlockActor* mba = (MovingBlockActor*)region.getBlockEntity(blockPos);
        if (mba->mWrappedBlock->getTypeName() != "minecraft:moving_block") { // 防止mb的mb导致的无限循环
            region.setBlock(blockPos, *mba->mWrappedBlock, 3, mba->mWrappedBlockActor, nullptr, BlockChangeContext());
            return origin(region, blockPos, region.getBlock(blockPos), random, resourceDropsContext, itemStacks);
        }
    }
    return origin(region, blockPos, block, random, resourceDropsContext, itemStacks);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropHook3,
    HookPriority::Normal,
    BlockType,
    &BlockType ::spawnResources,
    void,
    ::BlockSource&                region,
    ::BlockPos const&             pos,
    ::Block const&                block,
    ::IRandom&                    random,
    ::ResourceDropsContext const& resourceDropsContext,
    ::Actor const*                actorContext
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(region, pos, block, random, resourceDropsContext, actorContext));
    if (block.getTypeName() == "minecraft:moving_block") {
        MovingBlockActor* mba = (MovingBlockActor*)region.getBlockEntity(pos);
        if (mba->mWrappedBlock->getTypeName() != "minecraft:moving_block") { // 防止mb的mb导致的无限循环
            region.setBlock(pos, *mba->mWrappedBlock, 3, mba->mWrappedBlockActor, nullptr, BlockChangeContext());
            const Block& newBlock = region.getBlock(pos);
            return newBlock.mBlockType
                ->spawnResources(region, pos, newBlock, random, resourceDropsContext, actorContext);
        }
    }
    return origin(region, pos, block, random, resourceDropsContext, actorContext);
}

void bedrockDropHook(bool bl) { bl ? CoralFansDropHook1::hook() : CoralFansDropHook1::unhook(); }
void mbDropHook(bool bl) {
    if (bl) {
        CoralFansDropHook2::hook();
        CoralFansDropHook3::hook();
    } else {
        CoralFansDropHook2::unhook();
        CoralFansDropHook3::unhook();
    }
}
} // namespace coral_fans::functions
