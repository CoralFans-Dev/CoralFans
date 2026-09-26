#include "NoclipManager.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/helper/MainThreadExecutor.h"


#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"
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
    RETURN_IF_NOT_MAIN_THREAD(origin(gameType));
    origin(gameType);
    if (!isLoading() && gameType == GameType::Creative
        && CoralFans::getInstance().getConfigDb()->get(std::format("noclip.players.{}", this->getUuid().asString()))
               == "T") {
        CoralFans::getInstance().getSelf().getLogger().info("ServerPlayer::$setPlayerGameType triggered");
        NoclipManager::getInstance().enableNoclip(this);
    }
}
LL_TYPE_INSTANCE_HOOK(
    PlayerLoadHook2,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$load,
    bool,
    ::CompoundTag const& tag,
    ::DataLoadHelper&    dataLoadHelper
) {
    RETURN_IF_NOT_MAIN_THREAD(origin(tag, dataLoadHelper));
    auto ori = origin(tag, dataLoadHelper);
    if (ori && getPlayerGameType() == GameType::Creative
        && CoralFans::getInstance().getConfigDb()->get(std::format("noclip.players.{}", this->getUuid().asString()))
               == "T") {
        CoralFans::getInstance().getSelf().getLogger().info("ServerPlayer::$load triggered");
        auto& abilities = getAbilities();
        abilities.setAbility(AbilitiesIndex::Flying, true);
        abilities.setAbility(AbilitiesIndex::NoClip, true);
    }
    return ori;
}

void NoclipManager::hook(bool enable) {
    if (enable) {
        CoralFansNoClipHook::hook();
        PlayerLoadHook2::hook();
    } else {
        CoralFansNoClipHook::unhook();
        PlayerLoadHook2::unhook();
    }
}

void NoclipManager::clear() { this->handlingList.clear(); }

void NoclipManager::enableNoclip(Player* player) {
    if (!player) return;
    if (this->handlingList.contains(player->mName)) return;
    auto& abilities = player->getAbilities();
    if (abilities.getAbility(AbilitiesIndex::Flying).mValue->mBoolVal)
        player->setAbility(::AbilitiesIndex::NoClip, true);
    else {
        player->setAbility(::AbilitiesIndex::Flying, true);
        this->handlingList.emplace(player->mName);
        using namespace ll::chrono_literals;
        ll::coro::keepThis([playername = player->mName.get()]() -> ll::coro::CoroTask<> {
            co_await 3_tick;

            auto level = ll::service::getLevel();
            if (level.has_value()) [[likely]] {
                auto& noclipManager = NoclipManager::getInstance();
                if (auto it = noclipManager.handlingList.find(playername); it != noclipManager.handlingList.end())
                    [[likely]] {
                    Player* pl = level->getPlayer(playername);
                    if (pl) pl->setAbility(::AbilitiesIndex::NoClip, true);
                    noclipManager.handlingList.erase(it);
                }
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
