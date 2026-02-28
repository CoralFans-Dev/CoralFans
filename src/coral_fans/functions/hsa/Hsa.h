#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/chunk/LevelChunk.h"

#include <unordered_map>
#include <vector>

namespace coral_fans::functions {

// struct PairHash {
//     std::size_t operator()(const std::pair<ChunkPos, int>& p) const {
//         auto hash1 = std::hash<ChunkPos>{}(p.first);
//         auto hash2 = std::hash<int>{}(p.second);

//         return hash1 ^ (hash2 << 1);
//     }
// };

class HsaManager {
private:
    struct ChunkData {
        bsci::GeometryGroup::GeoId hsaGeoId       = {0};
        bsci::GeometryGroup::GeoId structureGeoId = {0};
    };

private:
    std::unordered_map<std::pair<ChunkPos, DimensionType>, ChunkData> mChunkDataMap;

public:
    bool hsaShow       = false;
    bool structureShow = false;

public:
    void drawChunkHsa(std::vector<::BlockPos>&, DimensionType, ChunkData&);
    void
    drawChunkStructure(entt::basic_storage<::br::ChunkBoundingBox, ::br::ChunkEntity, ::std::allocator<::br::ChunkBoundingBox>, void>&, DimensionType, ChunkData&);
    void drawChunk(DimensionType, ChunkPos);
    void tick();
    void remove();
    void runtimeRemove();
};

} // namespace coral_fans::functions