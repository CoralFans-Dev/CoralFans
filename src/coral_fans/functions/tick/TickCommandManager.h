#pragma once

#include "ll/api/event/ListenerBase.h"

#include <string>
#include <unordered_set>
#include <vector>

class Player;

namespace coral_fans::functions {

class TickCommandManager {
private:
    // 以玩家名称为键，记录开启了客户端 timescale 同步的玩家
    std::unordered_set<std::string>     mSyncedPlayers;
    std::vector<ll::event::ListenerPtr> mListeners;

    TickCommandManager() = default;

public:
    TickCommandManager(const TickCommandManager&)            = delete;
    TickCommandManager& operator=(const TickCommandManager&) = delete;

    [[nodiscard]] static TickCommandManager& getInstance() {
        static TickCommandManager instance;
        return instance;
    }

    [[nodiscard]] bool isSynced(std::string const& name) const { return mSyncedPlayers.contains(name); }

    // 开关指定玩家的客户端同步，返回新状态
    bool setSync(Player& player, bool enable);

    // tick rate 改变时调用：oldScale 为改变前的 mTimeScale，newScale 为新的 mTimeScale
    void applyRateChange(float newScale);
    // tick reset 时调用
    void applyReset();

    void hook(bool enable);
    void clear() { mSyncedPlayers.clear(); }

private:
    // 当前服务端 timescale（未运行时返回 1.0）
    static float currentScale();
    static void  sendSimTimeScalePacket(Player& player, float scale);
    // 以 multiplier 缩放玩家当前飞行速度，并同步到客户端
    static void setFlySpeedScale(Player& player, float multiplier);

public:
    void onPlayerJoin(Player& player);
    void onPlayerLeave(Player& player);
};

} // namespace coral_fans::functions
