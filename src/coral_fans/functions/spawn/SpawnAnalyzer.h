#pragma once

#include "ll/api/event/ListenerBase.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/dimension/DimensionType.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

class Level;
class Player;

namespace ll::event::inline world {
class SpawnedMobEvent;
}

namespace coral_fans::functions {

class SpawnAnalyzer {
    bool         mAnalyzing{false};
    DimensionType mDimensionId{};
    ChunkPos     mCenterChunkPos{0, 0};
    uint64_t     mTickCount{0};

    std::map<std::string, uint64_t> mSurfaceSpawnCounts;
    std::map<std::string, uint64_t> mCaveSpawnCounts;
    std::map<std::string, uint64_t> mSurfaceDensitySamples;
    std::map<std::string, uint64_t> mCaveDensitySamples;

    std::vector<ll::event::ListenerPtr> mListeners;

    SpawnAnalyzer() = default;

    void unsubscribe();
    void onMobSpawned(ll::event::world::SpawnedMobEvent& event);
    void onLevelTick(Level& level);

public:
    SpawnAnalyzer(SpawnAnalyzer const&)            = delete;
    SpawnAnalyzer& operator=(SpawnAnalyzer const&) = delete;

    static SpawnAnalyzer& getInstance();

    bool start(Player& player);
    bool stop();
    void clear();
    void shutdown();

    bool isAnalyzing() const { return mAnalyzing; }

    std::optional<std::string> buildResult() const;
};

} // namespace coral_fans::functions
