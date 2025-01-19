#include "coral_fans/functions/minerule/RemovePortalZombieCD.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/block/PortalBlock.h"


namespace coral_fans::functions {
LL_TYPE_STATIC_HOOK(
    CoralFansportalPortalZombieHook,
    ll::memory::HookPriority::Normal,
    PortalBlock,
    &PortalBlock::trySpawnPigZombie,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::PortalAxis      axis
) {
    Vec3 _pos = pos;
    while (_pos.y-- > 1) {
        if (region.canProvideSupport(_pos, 1, ::BlockSupportType(2))
            // && !region.isSolidBlockingBlock(_pos.x, _pos.y + 1, pos.z)
            // 源码中的判定条件，但是据我所知没有任何一个固体方块不给上方提供支持，所以源码中这条判定完全是多余的
        ) {
            if (axis == PortalAxis(2)) {
                _pos.x += 1.5f;
                _pos.z += 0.5f;
            } else {
                _pos.x += 0.5f;
                _pos.z += 1.5f;
            }
            _pos.y += 1.1f;
            ll::service::getLevel()
                ->getSpawner()
                .spawnMob(region, ActorDefinitionIdentifier(ActorType(68388)), nullptr, _pos, false, true, false);
            return;
        }
    }
}

void portal_spawn_hook(bool bl) {
    bl ? CoralFansportalPortalZombieHook::hook() : CoralFansportalPortalZombieHook ::unhook();
}
} // namespace coral_fans::functions