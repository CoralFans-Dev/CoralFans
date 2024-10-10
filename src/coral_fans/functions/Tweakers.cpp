#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/enums/AbilitiesIndex.h"
#include "mc/enums/GameType.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"

namespace coral_fans::functions {
// force open
LL_TYPE_INSTANCE_HOOK(
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
LL_TYPE_INSTANCE_HOOK(
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
LL_TYPE_INSTANCE_HOOK(
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
LL_TYPE_INSTANCE_HOOK(
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
LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersSafeExplodeHook,
    ll::memory::HookPriority::Normal,
    Explosion,
    &Explosion::explode,
    bool
) {
    if (coral_fans::mod().getConfigDb()->get("functions.global.safeexplode") != "true") return origin();
    else return false;
}

// fastdrop
LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersFastDropHook,
    ll::memory::HookPriority::Normal,
    Player,
    "?drop@Player@@UEAA_NAEBVItemStack@@_N@Z",
    bool,
    ItemStack const& item,
    bool             randomly
) {
    auto& modcfg = coral_fans::mod().getConfigDb();
    if (modcfg->get("functions.global.fastdrop") == "true"
        && modcfg->get(std::format("functions.players.{}.fastdrop", this->getUuid().asString())) == "true") {
        auto& inv  = this->getInventory();
        int   size = inv.getContainerSize();
        for (int i = 0; i < size; ++i) {
            const auto& itemi = inv.getItem(i);
            if (itemi.matchesItem(item))
                if (origin(itemi, randomly)) inv.setItem(i, ItemStack::EMPTY_ITEM);
        }
        this->refreshInventory();
        return false;
    } else return origin(item, randomly);
}

void hookTweakers(bool hook) {
    if (hook) {
        CoralFansTweakersForceOpenHook::hook();
        CoralFansTweakersForcePlaceHook::hook();
        CoralFansTweakersNoClipHook::hook();
        CoralFansTweakersDropperNoCostHook::hook();
        CoralFansTweakersSafeExplodeHook::hook();
        CoralFansTweakersFastDropHook::hook();
    } else {
        CoralFansTweakersForceOpenHook::unhook();
        CoralFansTweakersForcePlaceHook::unhook();
        CoralFansTweakersNoClipHook::unhook();
        CoralFansTweakersDropperNoCostHook::unhook();
        CoralFansTweakersSafeExplodeHook::unhook();
        CoralFansTweakersFastDropHook::unhook();
    }
}

} // namespace coral_fans::functions
