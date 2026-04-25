#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/MovingBlockActor.h"
#include <vector>

namespace coral_fans::functions {
constexpr float epsilon = 0.00001f; // 防止浮点数误差导致的错误

float myAdd(float a, float b) {
    float absA = std::abs(a);
    float absB = std::abs(b);

    if (absA > absB + epsilon) return a;
    if (absB > absA + epsilon) return b;

    // 当绝对值在误差范围内相等时：
    if ((a > 0 && b > 0) || (a < 0 && b < 0)) return a;
    return 0;
}

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

        Vec3 finalVelocity;
        finalVelocity.x = myAdd(velocity.x, engineVelocity.x);
        finalVelocity.y = myAdd(velocity.y, engineVelocity.y);
        finalVelocity.z = myAdd(velocity.z, engineVelocity.z);

        entity->mBuiltInComponents->mStateVectorComponent->mPosDelta = finalVelocity;
    }
}

void pistonCollisionHook(bool bl) { bl ? collisionHook::hook() : collisionHook::unhook(); }
} // namespace coral_fans::functions