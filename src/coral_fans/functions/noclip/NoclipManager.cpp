#include "NoclipManager.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/MySchedule.h"

#include "chrono"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "mc/server/ServerInstance.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/level/Level.h"
#include <thread>

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
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
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
        // my_schedule::MySchedule::getSchedule().add(
        //     [playername = player->mName.get()](int&, int&) {
        //         auto level = ll::service::getLevel();
        //         if (!level.has_value()) return false;
        //         Player* pl = nullptr;
        //         level->forEachPlayer([&pl, playername](Player& p) {
        //             if (p.mName.get() == playername) pl = &p;
        //             return false;
        //         });
        //         if (pl) pl->setAbility(::AbilitiesIndex::NoClip, true);
        //         return false;
        //     },
        //     3
        // );
        using namespace ll::chrono_literals;
        CoralFans::getInstance().getSelf().getLogger().info(*player->mName);
        // static std::string playername;
        // playername = player->mName.get();
        ll::coro::keepThis([playername = player->mName.get()]() -> ll::coro::CoroTask<> {
            co_await 150ms; // 等待0.15秒
            CoralFans::getInstance().getSelf().getLogger().info(playername);

            auto level = ll::service::getLevel();
            CoralFans::getInstance().getSelf().getLogger().info("c");
            if (level.has_value()) {
                Player* pl = nullptr;
                level->forEachPlayer([&pl, &playername](Player& p) {
                    CoralFans::getInstance().getSelf().getLogger().info("d");
                    if (*p.mName == playername) {
                        CoralFans::getInstance().getSelf().getLogger().info("ddd");
                        pl = &p;
                    }
                    CoralFans::getInstance().getSelf().getLogger().info("d");
                    CoralFans::getInstance().getSelf().getLogger().info(*p.mName);
                    CoralFans::getInstance().getSelf().getLogger().info("d");
                    return false;
                });
                CoralFans::getInstance().getSelf().getLogger().info("d");
                // CoralFans::getInstance().getSelf().getLogger().info(playername);
                CoralFans::getInstance().getSelf().getLogger().info("d");
                if (pl) {
                    CoralFans::getInstance().getSelf().getLogger().info("e");
                    pl->setAbility(::AbilitiesIndex::NoClip, true);
                }
            }
        })
            .launch(
                ll::thread::ServerThreadExecutor::getDefault(),
                [](ll::Expected<> result) {
                    if (!result) {
                        // 输出错误信息
                        CoralFans::getInstance().getSelf().getLogger().error(
                            "Coroutine failed: {}",
                            result.error().message()
                        );

                    } else {
                        CoralFans::getInstance().getSelf().getLogger().info("Coroutine completed successfully");
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
