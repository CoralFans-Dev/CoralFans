#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/enums/AbilitiesIndex.h"
#include "mc/enums/GameType.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"

namespace coral_fans::functions {
// force open
LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersForceOpenHook,
    ll::memory::HookPriority::Normal,
    ChestBlockActor,
    &ChestBlockActor::canOpen,
    bool,
    BlockSource& region
) {
    if (coral_fans::mod().getConfigDb()->get("functions.global.forceopen") == "true") return true;
    else return origin(region);
}

// force place
LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersForcePlaceHook,
    ll::memory::HookPriority::Normal,
    BlockSource,
    &BlockSource::mayPlace,
    bool,
    Block const&    block,
    BlockPos const& pos,
    uchar           face,
    Actor*          placer,
    bool            ignoreEntities,
    Vec3            clickPos
) {
    const auto type = coral_fans::mod().getConfigDb()->get("functions.global.forceplace");
    if (type == "entity") return origin(block, pos, face, placer, true, clickPos);
    else if (type == "all") return true;
    else return origin(block, pos, face, placer, ignoreEntities, clickPos);
}

// noclip
LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersNoClipHook,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    "?setPlayerGameType@ServerPlayer@@UEAAXW4GameType@@@Z",
    void,
    ::GameType gameType
) {
    origin(gameType);
    if (gameType == ::GameType::Creative && coral_fans::mod().getConfigDb()->get("functions.global.noclip") == "true"
        && coral_fans::mod().getConfigDb()->get(std::format("functions.players.{}.noclip", this->getUuid().asString()))
               == "true")
        this->setAbility(::AbilitiesIndex::NoClip, true);
}

// droppernocost
LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersDropperNoCostHook,
    ll::memory::HookPriority::Normal,
    Container,
    "?removeItem@Container@@UEAAXHH@Z",
    void,
    int slot,
    int count
) {
    if (coral_fans::mod().getConfigDb()->get("functions.global.droppernocost") != "true") origin(slot, count);
}

// safeexplode
LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersSafeExplodeHook,
    ll::memory::HookPriority::Normal,
    Explosion,
    &Explosion::explode,
    bool
) {
    if (coral_fans::mod().getConfigDb()->get("functions.global.safeexplode") != "true") return origin();
    else return false;
}
} // namespace coral_fans::functions
