#include "coral_fans/functions/minerule/drophook.h"
#include "ll/api/memory/Hook.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/nbt/CompoundTag.h"
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
#include "mc/world/level/block/components/BlockComponentStorage.h"


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

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropHook2,
    ll::memory::HookPriority::Normal,
    Block,
    &Block::onExploded,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor*          entitySource
) {
    if (this->getTypeName() == "minecraft:moving_block") {
        MovingBlockActor* mba = (MovingBlockActor*)region.getBlockEntity(pos);
        region.setBlock(pos, mba->getWrappedBlock(), 3, mba->mWrappedBlockActor, nullptr, nullptr);
        const Block&           bl      = region.getBlock(pos);
        ResourceDropsContext   tem     = ResourceDropsContext::fromExplosion(region, 1.0, BlockPos(0, 0, 0));
        Randomize              ran     = Randomize(tem.getRandom());
        std::vector<ItemStack> drops   = bl.getLegacyBlock().getResourceDrops(bl, ran, tem).mItems;
        Vec3                   _pos    = Vec3(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5);
        auto&                  spawner = ll::service::getLevel()->getSpawner();
        for (auto i : drops) {
            spawner.spawnItem(region, i, 0, _pos, region.getDimensionId());
        }
    }
    return origin(region, pos, entitySource);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropHook3,
    HookPriority::Normal,
    Block,
    &Block::spawnResources,
    void,
    ::BlockSource&                region,
    ::BlockPos const&             pos,
    ::Randomize&                  randomize,
    ::ResourceDropsContext const& resourceDropsContext
) {
    if (this->getTypeName() == "minecraft:moving_block") {
        MovingBlockActor* mba = (MovingBlockActor*)region.getBlockEntity(pos);
        region.setBlock(pos, *mba->mWrappedBlock, 3, mba->mWrappedBlockActor, nullptr, nullptr);
        const Block& newBlock = region.getBlock(pos);
        return newBlock.spawnResources(region, pos, randomize, resourceDropsContext);
    }
    return origin(region, pos, randomize, resourceDropsContext);
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
