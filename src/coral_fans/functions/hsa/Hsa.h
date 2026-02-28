#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/chunk/ChunkBoundingBox.h"
#include "mc/world/level/chunk/ChunkEntity.h"

#include <unordered_map>
#include <vector>

namespace coral_fans::functions {

struct PairHash {
    std::size_t operator()(const std::pair<ChunkPos, DimensionType>& p) const {
        auto hash1 = std::hash<ChunkPos>{}(p.first);
        auto hash2 = std::hash<DimensionType>{}(p.second);

        return hash1 ^ (hash2 << 1);
    }
};

class HsaManager {
private:
    struct ChunkData {
        bsci::GeometryGroup::GeoId hsaGeoId       = {0};
        bsci::GeometryGroup::GeoId structureGeoId = {0};
        bool                       freshed        = true;
    };

private:
    std::unordered_map<std::pair<ChunkPos, DimensionType>, ChunkData, PairHash> mChunkDataMap;

private:
    int  tickCounter              = 1;
    int  runTimeRemoveTickCounter = 1;
    bool hsaShow                  = false;
    bool structureShow            = false;

public:
    void setHsaShow(bool show);
    void setStructureShow(bool show);
    bool getHsaShow();
    bool getStructureShow();

public:
    void drawChunkHsa(std::vector<::BlockPos>&, DimensionType, ChunkData&);
    void drawChunkStructure(
        entt::basic_storage<::br::ChunkBoundingBox, ::br::ChunkEntity, ::std::allocator<::br::ChunkBoundingBox>, void>&,
        DimensionType,
        ChunkData&,
        ChunkPos
    );
    void                  draw();
    void                  tick();
    void                  remove();
    void                  runtimeRemove();
    std::vector<BlockPos> listChunkHsa(BlockSource&, ChunkPos);
    std::vector<AABB>     listChunkStructure(BlockSource&, ChunkPos);
};

} // namespace coral_fans::functions