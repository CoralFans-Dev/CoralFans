#include "coral_fans/functions/minerule/drophook.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockLegacy.h"
#include "mc/world/level/block/CachedComponentData.h"
#include "mc/world/level/block/ResourceDrops.h"
#include "mc/world/level/block/actor/MovingBlockActor.h"
#include "mc/world/level/block/components/BlockComponentDirectData.h"
#include "mc/world/level/block/components/BlockComponentStorage.h"


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
    if (DropHookManager::getInstance().bedrockDrop && block.getTypeName() == "minecraft:bedrock") {
        std::vector<ItemStack> a{ItemStack("bedrock", 1, 0, nullptr)};
        return ResourceDrops(a);
    } else if (DropHookManager::getInstance().mbDrop && block.getTypeName() == "minecraft:moving_block") {
        DropHookManager::getInstance().ram          = &randomize;
        DropHookManager::getInstance().dropsContext = &resourceDropsContext;
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
        const Block&           bl = region.getBlock(pos);
        std::vector<ItemStack> drops =
            bl.getLegacyBlock()
                .getResourceDrops(bl, *DropHookManager::getInstance().ram, *DropHookManager::getInstance().dropsContext)
                .mItems;
        for (auto i : drops) {
            ll::service::getLevel()->getSpawner().spawnItem(region, i, 0, pos, region.getDimensionId());
        }
    }
    return origin(region, pos, entitySource);
}

void dropHook() {
    if (DropHookManager::getInstance().mbDrop) {
        CoralFansDropHook1::hook();
        CoralFansDropHook2::hook();
    } else if (DropHookManager::getInstance().bedrockDrop) {
        CoralFansDropHook1::hook();
        CoralFansDropHook2::unhook();
    } else {
        CoralFansDropHook1::unhook();
        CoralFansDropHook2::unhook();
    }
}
} // namespace coral_fans::functions