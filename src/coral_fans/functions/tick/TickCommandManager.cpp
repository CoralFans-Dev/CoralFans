#include "coral_fans/functions/tick/TickCommandManager.h"

#include "coral_fans/CoralFans.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/shared_types/legacy/LevelEvent.h"
#include "mc/network/packet/LevelEventPacket.h"
#include "mc/network/packet/UpdateAbilitiesPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/util/Timer.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/level/Level.h"

#include "ll/api/memory/Hook.h"

#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


namespace coral_fans::functions {

namespace {
constexpr float kDefaultFlySpeed         = 0.05f;
constexpr float kDefaultVerticalFlySpeed = 1.0f;
constexpr float kMinScale                = 1e-6f;
} // namespace

float TickCommandManager::currentScale() {
    if (auto mc = ll::service::getMinecraft()) return mc->mSimTimer.mTimeScale;
    return 1.0f;
}

void TickCommandManager::sendSimTimeScalePacket(Player& player, float scale) {
    LevelEventPacket pkt;
    pkt.mEventId = static_cast<int>(SharedTypes::Legacy::LevelEvent::SimTimeScale);
    pkt.mPos->x  = scale;
    pkt.sendTo(player);
}

void TickCommandManager::setFlySpeedScale(Player& player, float multiplier) {
    auto& abilities = player.getAbilities();
    abilities.setAbility(AbilitiesIndex::FlySpeed, kDefaultFlySpeed * multiplier);
    abilities.setAbility(AbilitiesIndex::VerticalFlySpeed, kDefaultVerticalFlySpeed * multiplier);
    UpdateAbilitiesPacket{player.getOrCreateUniqueID(), abilities}.sendTo(player);
}

bool TickCommandManager::setSync(Player& player, bool enable) {
    const auto  name  = player.getRealName();
    const float scale = currentScale();
    if (enable) {
        mSyncedPlayers.insert(name);
        sendSimTimeScalePacket(player, scale);
        if (scale > kMinScale) setFlySpeedScale(player, 1.0f / scale);
    } else {
        mSyncedPlayers.erase(name);
        sendSimTimeScalePacket(player, 1.0f);
        if (scale > kMinScale) setFlySpeedScale(player, 1.0f);
    }
    return enable;
}

void TickCommandManager::applyRateChange(float newScale) {
    if (mSyncedPlayers.empty()) return;
    auto level = ll::service::getLevel();
    if (!level) return;
    for (auto const& name : mSyncedPlayers) {
        auto* player = level->getPlayer(name);
        if (!player) continue;
        sendSimTimeScalePacket(*player, 1.0f);
        sendSimTimeScalePacket(*player, newScale);
        if (newScale > kMinScale) setFlySpeedScale(*player, 1.0f / newScale);
    }
}

void TickCommandManager::applyReset() {
    if (mSyncedPlayers.empty()) return;
    auto level = ll::service::getLevel();
    if (!level) return;
    for (auto const& name : mSyncedPlayers) {
        auto* player = level->getPlayer(name);
        if (!player) continue;
        sendSimTimeScalePacket(*player, 1.0f);
        setFlySpeedScale(*player, 1.0f);
    }
}

void TickCommandManager::onPlayerJoin(Player& player) {
    float scale;
    // 重进世界时客户端 timer 已被重置，若玩家此前开启了同步则重新同步
    if (isSynced(player.getRealName())) scale = currentScale();
    else scale = 1.0f;
    sendSimTimeScalePacket(player, scale);
    if (scale > kMinScale) {
        auto& abilities = player.getAbilities();
        abilities.setAbility(AbilitiesIndex::FlySpeed, kDefaultFlySpeed / scale);
        abilities.setAbility(AbilitiesIndex::VerticalFlySpeed, kDefaultVerticalFlySpeed / scale);
    }
}

void TickCommandManager::onPlayerLeave(Player& player) {
    // 断开连接是客户端行为，无法在断开前发送复位包，只能恢复服务端的飞行速度
    if (!isSynced(player.getRealName())) return;
    CoralFans::getInstance().getSelf().getLogger().info("player leave");
    player.getAbilities().setAbility(AbilitiesIndex::FlySpeed, kDefaultFlySpeed);
    player.getAbilities().setAbility(AbilitiesIndex::VerticalFlySpeed, kDefaultVerticalFlySpeed);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerLoadHook,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$load,
    bool,
    ::CompoundTag const& tag,
    ::DataLoadHelper&    dataLoadHelper
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(tag, dataLoadHelper);
#endif
    auto ori = origin(tag, dataLoadHelper);
    if (ori) TickCommandManager::getInstance().onPlayerJoin(*this);
    return ori;
}

void TickCommandManager::hook(bool enable) {
    auto& bus = ll::event::EventBus::getInstance();
    if (enable) {
        PlayerLoadHook::hook();
        if (mListeners.empty()) {
            mListeners.emplace_back(bus.emplaceListener<ll::event::player::PlayerDisconnectEvent>(
                [](ll::event::player::PlayerDisconnectEvent& event) { getInstance().onPlayerLeave(event.self()); }
            ));
        }
    } else if (!enable) {
        PlayerLoadHook::unhook();
        for (auto const& listener : mListeners) bus.removeListener(listener);
        mListeners.clear();
    }
}

} // namespace coral_fans::functions
