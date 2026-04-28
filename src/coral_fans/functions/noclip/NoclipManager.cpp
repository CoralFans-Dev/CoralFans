#include "NoclipManager.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/helper/MainThreadExecutor.h"


#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/level/Level.h"


#ifdef LL_PLAT_C
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


namespace coral_fans::functions {

LL_TYPE_INSTANCE_HOOK(
    CoralFansNoClipHook,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$setPlayerGameType,
    void,
    ::GameType gameType
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(gameType);
#endif
    origin(gameType);
    if (gameType == ::GameType::Creative
        && CoralFans::getInstance().getConfigDb()->get(std::format("noclip.players.{}", this->getUuid().asString()))
               == "T") {
        NoclipManager::getInstance().enableNoclip(this);
    }
}

void NoclipManager::hook(bool enable) {
    if (enable) {
        CoralFansNoClipHook::hook();
    } else {
        CoralFansNoClipHook::unhook();
    }
}

void NoclipManager::enableNoclip(Player* player) {
    if (!player) return;
    auto& abilities = player->getAbilities();
    if (abilities.getAbility(AbilitiesIndex::Flying).mValue->mBoolVal)
        player->setAbility(::AbilitiesIndex::NoClip, true);
    else {
        player->setAbility(::AbilitiesIndex::Flying, true);
        using namespace ll::chrono_literals;
        ll::coro::keepThis([playername = player->mName.get()]() -> ll::coro::CoroTask<> {
            co_await 3_tick;

            auto level = ll::service::getLevel();
            if (level.has_value()) {
                Player* pl = level->getPlayer(playername);
                if (pl) pl->setAbility(::AbilitiesIndex::NoClip, true);
            }
        })
            .launch(
                helper::thread::MainThreadExecutor::getDefault(),
                [](ll::Expected<> result) {
                    if (!result) {
                        // 输出错误信息
                        CoralFans::getInstance().getSelf().getLogger().error(
                            "Coroutine failed: {}",
                            result.error().message()
                        );
                    }
                }

            );
    }
}

void NoclipManager::disableNoclip(Player* player) {
    if (!player) return;
    player->setAbility(::AbilitiesIndex::NoClip, false);
    player->setAbility(::AbilitiesIndex::Flying, true);
}

} // namespace coral_fans::functions
