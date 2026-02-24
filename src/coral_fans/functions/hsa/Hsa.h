#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"

#include <unordered_map>
#include <vector>

namespace coral_fans::functions {

struct PairHash {
    std::size_t operator()(const std::pair<ChunkPos, int>& p) const {
        auto hash1 = std::hash<ChunkPos>{}(p.first);
        auto hash2 = std::hash<int>{}(p.second);

        return hash1 ^ (hash2 << 1);
    }
};

class HsaManager {
private:
    std::unordered_map<std::pair<ChunkPos, int>, std::pair<std::vector<bsci::GeometryGroup::GeoId>, bool>, PairHash>
        mParticleMap;

public:
    bool mShow;

public:
    void drawHsa();
    void tick();
    void remove();
    void runtimeRemove();
};

} // namespace coral_fans::functions