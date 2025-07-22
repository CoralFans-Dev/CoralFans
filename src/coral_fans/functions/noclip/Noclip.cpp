#include "Noclip.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/MySchedule.h"
#include "ll/api/memory/Hook.h"
#include "mc/server/ServerPlayer.h"


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    CoralFansNoClipHook,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$setPlayerGameType,
    void,
    ::GameType gameType
) {
    origin(gameType);
    if (gameType == ::GameType::Creative
        && coral_fans::mod().getConfigDb()->get(std::format("noclip.players.{}", this->getUuid().asString())) == "T") {
        this->setAbility(::AbilitiesIndex::Flying, true);
        my_schedule::MySchedule::getSchedule().add(3, [this]() { this->setAbility(::AbilitiesIndex::NoClip, true); });
    }
}

void noclipHook(bool isopen) { isopen ? CoralFansNoClipHook::hook() : CoralFansNoClipHook::unhook(); }
} // namespace coral_fans::functions