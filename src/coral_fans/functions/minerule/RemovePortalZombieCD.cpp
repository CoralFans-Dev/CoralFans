#include "MineruleManager.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/SpawnChecks.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/PortalShape.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockSupportType.h"
#include "mc/world/level/block/PortalBlock.h"
#include "mc/world/level/block/VanillaStates.h "


namespace coral_fans::functions {
// LL_TYPE_STATIC_HOOK(
//     CoralFansportalPortalZombieHook,
//     ll::memory::HookPriority::Normal,
//     PortalBlock,
//     &PortalBlock::trySpawnPigZombie,
//     void,
//     ::BlockSource&    region,
//     ::BlockPos const& pos,
//     ::PortalAxis      axis
// ) {
// #ifdef LL_PLAT_C
//     if (auto serverInstance = ll::service::getServerInstance();
//         !serverInstance
//         || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
//         return origin(region, pos, axis);
// #endif
//     Vec3 _pos = pos;
//     while (_pos.y-- > 1) {
//         auto& block = region.getBlock(_pos);
//         if (block.mBlockType->canProvideSupport(block, 1, BlockSupportType::Any)
//             // && !region.isSolidBlockingBlock(_pos.x, _pos.y + 1, pos.z)
//             // 源码中的判定条件，但是据我所知没有任何一个固体方块不给上方提供支持，所以源码中这条判定完全是多余的
//         ) {
//             if (axis == PortalAxis::Z) {
//                 _pos.x += 1.5f;
//                 _pos.z += 0.5f;
//             } else if (axis == PortalAxis::X) {
//                 _pos.x += 0.5f;
//                 _pos.z += 1.5f;
//             }
//             _pos.y += 1.1f;
//             ll::service::getLevel()->getSpawner().spawnMob(
//                 region,
//                 ActorDefinitionIdentifier("minecraft:zombie_pigman"),
//                 nullptr,
//                 _pos,
//                 false,
//                 true,
//                 false
//             );
//             return;
//         }
//     }
// }

void trySpawnPigZombieWithoutCD(::BlockSource& region, ::BlockPos const& pos, ::PortalAxis axis) {
    Vec3 _pos = pos;
    while (_pos.y-- > 1) {
        auto& block = region.getBlock(_pos);
        if (
            block.mBlockType->canProvideSupport(block, 1, BlockSupportType::Any)
            // && !region.isSolidBlockingBlock(_pos.x, _pos.y + 1, pos.z)
            // 源码中的判定条件，但是据我所知没有任何一个固体方块不给上方提供支持，所以源码中这条判定完全是多余的
        ) {
            if (axis == PortalAxis::Z) {
                _pos.x += 1.5f;
                _pos.z += 0.5f;
            } else if (axis == PortalAxis::X) {
                _pos.x += 0.5f;
                _pos.z += 1.5f;
            }
            _pos.y += 1.1f;
            ll::service::getLevel()->getSpawner().spawnMob(
                region,
                ActorDefinitionIdentifier("minecraft:zombie_pigman"),
                nullptr,
                _pos,
                false,
                true,
                false
            );
            return;
        }
    }
}

LL_TYPE_STATIC_HOOK(
    portalPortalZombieHook,
    ll::memory::HookPriority::Normal,
    SpawnChecks,
    &SpawnChecks::canSpawnPigZombieFromPortal,
    bool,
    ::Dimension const& dimension,
    ::IRandom&         random
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(dimension, random));
    RemovePortalZombieCDHelper::getInstance().spawn = origin(dimension, random);
    return false;
}

LL_TYPE_STATIC_HOOK(
    portalPortalZombieHook2,
    ll::memory::HookPriority::Normal,
    PortalBlock,
    &PortalBlock::_tick,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    RETURN_IF_NOT_MAIN_THREAD(return origin(region, pos, random));
    origin(region, pos, random);
    auto& helper = RemovePortalZombieCDHelper::getInstance();
    if (helper.spawn) {
        helper.spawn     = false;
        auto& block      = region.getBlock(pos);
        auto  portalAxis = block.getState<int>(VanillaStates::PortalAxis());
        if (portalAxis) trySpawnPigZombieWithoutCD(region, pos, static_cast<PortalAxis>(*portalAxis));
    }
}

void portalSpawnHook(bool bl) {
    if (bl) {
        portalPortalZombieHook::hook();
        portalPortalZombieHook2::hook();
    } else {
        portalPortalZombieHook::unhook();
        portalPortalZombieHook2::unhook();
    }
}
} // namespace coral_fans::functions