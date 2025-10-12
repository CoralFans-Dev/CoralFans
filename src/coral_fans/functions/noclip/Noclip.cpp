#include "Noclip.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/MySchedule.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/level/Level.h"


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
        my_schedule::MySchedule::getSchedule().add(
            [playername = this->mName.get()](int&, int&) {
                auto level = ll::service::getLevel();
                if (!level.has_value()) return false;
                Player* pl = nullptr;
                level->forEachPlayer([&pl, playername](Player& player) {
                    if (player.mName.get() == playername) pl = &player;
                    return false;
                });
                if (pl) pl->setAbility(::AbilitiesIndex::NoClip, true);
                return false;
            },
            3
        );
    }
}

void noclipHook(bool isopen) { isopen ? CoralFansNoClipHook::hook() : CoralFansNoClipHook::unhook(); }
} // namespace coral_fans::functions