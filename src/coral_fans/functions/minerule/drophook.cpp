#include "coral_fans/functions/minerule/drophook.h"
#include "ll/api/memory/Hook.h"
#include "mc/scripting/modules/minecraft/events/ScriptBlockGlobalEventListener.h"
#include "mc/util/Randomize.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockLegacy.h"
#include "mc/world/level/block/CachedComponentData.h"
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
    BlockLegacy,
    &BlockLegacy::getResourceDrops,
    ResourceDrops,
    ::Block const&                block,
    ::Randomize&                  randomize,
    ::ResourceDropsContext const& resourceDropsContext
) {
    if (block.getTypeName() == "minecraft:bedrock") {
        std::vector<ItemStack> a{ItemStack("bedrock", 1, 0, nullptr)};
        return ResourceDrops(a);
    }
    return origin(block, randomize, resourceDropsContext);
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
    ::Randomize&                                         randomize,
    ::ResourceDropsContext const&                        resourceDropsContext,
    ::std::vector<::std::pair<::ItemStack, ::BlockPos>>& itemStacks
) {
    if (block.getTypeName() == "minecraft:moving_block") {
        MovingBlockActor* mba = (MovingBlockActor*)region.getBlockEntity(blockPos);
        if (mba->mWrappedBlock->getTypeName() != "minecraft:moving_block") { // 防止mb的mb导致的无限循环
            region.setBlock(blockPos, *mba->mWrappedBlock, 3, mba->mWrappedBlockActor, nullptr, nullptr);
            return origin(region, blockPos, region.getBlock(blockPos), randomize, resourceDropsContext, itemStacks);
        }
    }
    return origin(region, blockPos, block, randomize, resourceDropsContext, itemStacks);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropHook3,
    HookPriority::Normal,
    BlockLegacy,
    &BlockLegacy ::spawnResources,
    void,
    ::BlockSource&                region,
    ::BlockPos const&             pos,
    ::Block const&                block,
    ::Randomize&                  randomize,
    ::ResourceDropsContext const& resourceDropsContext
) {
    if (block.getTypeName() == "minecraft:moving_block") {
        MovingBlockActor* mba = (MovingBlockActor*)region.getBlockEntity(pos);
        region.setBlock(pos, *mba->mWrappedBlock, 3, mba->mWrappedBlockActor, nullptr, nullptr);
        const Block& newBlock = region.getBlock(pos);
        return newBlock.mLegacyBlock->spawnResources(region, pos, newBlock, randomize, resourceDropsContext);
    }
    return origin(region, pos, block, randomize, resourceDropsContext);
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
