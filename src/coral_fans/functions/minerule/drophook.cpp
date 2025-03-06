#include "coral_fans/functions/minerule/drophook.h"
#include "ll/api/memory/Hook.h"
#include "mc/scripting/modules/minecraft/events/ScriptBlockGlobalEventListener.h"
#include "mc/util/Randomize.h"
#include "mc/world/item/ItemStack.h"
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

#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockPos.h"


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
        region.setBlock(blockPos, *mba->mWrappedBlock, 3, mba->mWrappedBlockActor, nullptr, nullptr);
        return origin(region, blockPos, region.getBlock(blockPos), randomize, resourceDropsContext, itemStacks);
    }
    return origin(region, blockPos, block, randomize, resourceDropsContext, itemStacks);
}

void bedrockDropHook(bool bl) { bl ? CoralFansDropHook1::hook() : CoralFansDropHook1::unhook(); }
void mbDropHook(bool bl) { bl ? CoralFansDropHook2::hook() : CoralFansDropHook2::unhook(); }
} // namespace coral_fans::functions

// #include "coral_fans/functions/minerule/Drophook.h"
// #include "ll/api/memory/Hook.h"
// #include "ll/api/service/Bedrock.h"
// #include "mc/deps/core/math/Vec3.h"
// #include "mc/scripting/modules/minecraft/events/ScriptBlockGlobalEventListener.h"
// #include "mc/world/item/ItemStack.h"
// #include "mc/world/level/BlockPos.h"
// #include "mc/world/level/Level.h"
// #include "mc/world/level/Spawner.h"
// #include "mc/world/level/block/Block.h"
// #include "mc/world/level/block/BlockLegacy.h"
// #include "mc/world/level/block/ResourceDrops.h"
// #include "mc/world/level/block/actor/MovingBlockActor.h"
// #include "mc/world/level/dimension/Dimension.h"


// #include <mc/world/level/BlockSource.h>


// namespace coral_fans::functions {
// LL_TYPE_INSTANCE_HOOK(
//     CoralFansDropHook1,
//     ll::memory::HookPriority::Normal,
//     BlockLegacy,
//     &BlockLegacy::getResourceDrops,
//     ResourceDrops,
//     ::Block const&                block,
//     ::Randomize&                  randomize,
//     ::ResourceDropsContext const& resourceDropsContext
// ) {
//     if (DropHookManager::getInstance().bedrockDrop && block.getTypeName() == "minecraft:bedrock") {
//         std::vector<ItemStack> a{ItemStack("bedrock", 1, 0, nullptr)};
//         return ResourceDrops(a);
//     } else if (DropHookManager::getInstance().mbDrop && block.getTypeName() == "minecraft:moving_block") {
//         DropHookManager::getInstance().ram          = &randomize;
//         DropHookManager::getInstance().dropsContext = &resourceDropsContext;
//     }
//     return origin(block, randomize, resourceDropsContext);
// }

// LL_TYPE_INSTANCE_HOOK(
//     CoralFansDropHook2,
//     HookPriority::Normal,
//     ScriptModuleMinecraft::ScriptBlockGlobalEventListener,
//     &ScriptBlockGlobalEventListener::$onBlockExploded,
//     EventResult,
//     Dimension&      dimension,
//     BlockPos const& blockPos,
//     Block const&    destroyedBlock,
//     Actor*          source
// ) {
//     if (destroyedBlock.getTypeName() == "minecraft:moving_block") {
//         BlockSource&      region           = dimension.getBlockSourceFromMainChunkSource();
//         MovingBlockActor* mba              = (MovingBlockActor*)region.getBlockEntity(blockPos);
//         auto              mWrappedBlockTag = *mba->mWrappedBlock->mSerializationId;
//         auto              newBlock         = Block ::tryGetFromRegistry(mWrappedBlockTag);
//         region.setBlock(blockPos, newBlock, 3, mba->mWrappedBlockActor, nullptr, nullptr);
//         const Block&           bl = region.getBlock(blockPos);
//         std::vector<ItemStack> drops =
//             bl.getLegacyBlock()
//                 .getResourceDrops(bl, *DropHookManager::getInstance().ram,
//                 *DropHookManager::getInstance().dropsContext) .mItems;
//         Vec3  _pos    = Vec3(blockPos.x + 0.5, blockPos.y + 0.5, blockPos.z + 0.5);
//         auto& spawner = ll::service::getLevel()->getSpawner();
//         for (auto i : drops) {
//             spawner.spawnItem(region, i, 0, _pos, region.getDimensionId());
//         }
//     }
//     return origin(dimension, blockPos, destroyedBlock, source);
// }

// void dropHook() {
//     if (DropHookManager::getInstance().mbDrop) {
//         CoralFansDropHook1::hook();
//         CoralFansDropHook2::hook();
//     } else if (DropHookManager::getInstance().bedrockDrop) {
//         CoralFansDropHook1::hook();
//         CoralFansDropHook2::unhook();
//     } else {
//         CoralFansDropHook1::unhook();
//         CoralFansDropHook2::unhook();
//     }
// }
// } // namespace coral_fans::functions