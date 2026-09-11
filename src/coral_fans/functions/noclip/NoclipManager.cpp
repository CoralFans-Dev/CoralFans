#include "NoclipManager.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/helper/MainThreadExecutor.h"


#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/SetLocalPlayerAsInitializedPacket.h"
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
    if (gameType == GameType::Creative
        && CoralFans::getInstance().getConfigDb()->get(std::format("noclip.players.{}", this->getUuid().asString()))
               == "T") {
        NoclipManager::getInstance().enableNoclip(this);
        CoralFans::getInstance().getSelf().getLogger().info("PlayerJoinEventHook handled");
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerJoinEventHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    NetworkIdentifier const&                 identifier,
    SetLocalPlayerAsInitializedPacket const& packet
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(identifier, packet);
#endif
    if (auto player = thisFor<NetEventCallback>()->_getServerPlayer(identifier, packet.mSenderSubId);
        player && player->getPlayerGameType() == GameType::Creative
        && CoralFans::getInstance().getConfigDb()->get(std::format("noclip.players.{}", player->getUuid().asString()))
               == "T") {
        CoralFans::getInstance().getSelf().getLogger().info("PlayerJoinEventHook handled");
        NoclipManager::getInstance().enableNoclip(player);
    }
    origin(identifier, packet);
}

void NoclipManager::hook(bool enable) {
    if (enable) {
        CoralFansNoClipHook::hook();
        PlayerJoinEventHook::hook();
    } else {
        CoralFansNoClipHook::unhook();
        PlayerJoinEventHook::unhook();
    }
}

void NoclipManager::clear() { this->handlingList.clear(); }

void NoclipManager::enableNoclip(Player* player, bool forceDelay) {
    if (!player) return;
    if (this->handlingList.contains(player->mName)) return;
    auto& abilities = player->getAbilities();
    if (!abilities.getAbility(AbilitiesIndex::Flying).mValue->mBoolVal) {
        player->setAbility(::AbilitiesIndex::Flying, true);
        forceDelay = true;
    }

    if (!forceDelay) player->setAbility(::AbilitiesIndex::NoClip, true);
    else {
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
