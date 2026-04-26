#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/MovingBlockActor.h"
#include <vector>

namespace coral_fans::functions {

LL_TYPE_INSTANCE_HOOK(
    collisionHook,
    HookPriority::Normal,
    MovingBlockActor,
    &MovingBlockActor::moveCollidedEntities,
    void,
    ::PistonBlockActor& pistonBlock,
    ::BlockSource&      region
) {
    const Block* wrappedBlock = this->mWrappedBlock;

    if (!((uint64)wrappedBlock->mBlockType->mProperties & (uint64)BlockProperty::Slime))
        return origin(pistonBlock, region);

    auto [aabb1, aabb2] = this->_getWrappedBlockCollisionShapes(region);
    AABB combinedAABB   = aabb1;
    combinedAABB.merge(aabb2);

    struct EntityNapshot {
        Actor* entity;
        Vec3   velocity;
    };
    std::vector<EntityNapshot> entitySnapshots;

    auto entitySpan = region.fetchEntities(nullptr, combinedAABB, false, false);
    for (Actor* entity : entitySpan) {
        Vec3 velocity = entity->getPosDelta();
        entitySnapshots.push_back({entity, velocity});
    }

    origin(pistonBlock, region);

    for (auto [entity, velocity] : entitySnapshots) {
        if (!entity) gsl::details::terminate();

        Vec3 engineVelocity = entity->getPosDelta();
        if (engineVelocity == velocity) continue;

        Vec3 finalVelocity = velocity;
        if (engineVelocity.x != 0.0f) finalVelocity.x = engineVelocity.x;
        if (engineVelocity.y != 0.0f) finalVelocity.y = engineVelocity.y;
        if (engineVelocity.z != 0.0f) finalVelocity.z = engineVelocity.z;

        entity->mBuiltInComponents->mStateVectorComponent->mPosDelta = finalVelocity;
    }
}

void pistonCollisionHook(bool bl) { bl ? collisionHook::hook() : collisionHook::unhook(); }
} // namespace coral_fans::functions