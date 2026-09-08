#include "coral_fans/functions/spawn/Spawn.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/minerule/MineruleManager.h"

#include "ll/api/service/Bedrock.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/biome/MobSpawnerData.h"
#include "mc/world/level/biome/SpawnConditions.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"


#include <algorithm>
#include <stdexcept>

namespace coral_fans::functions {
namespace {

constexpr size_t kSpawnSurfaceIdx     = 1;
constexpr size_t kSpawnUndergroundIdx = 0;

BedrockSpawner& getBedrockSpawner() {
    if (auto level = ll::service::getLevel()) return static_cast<BedrockSpawner&>(level->getSpawner());
    throw std::runtime_error("level is not available");
}

template <typename T>
SpawnDensityCounts toSpawnDensityCounts(const T (&surface)[7], const T (&underground)[7]) {
    SpawnDensityCounts counts;
    std::copy_n(surface, 7, counts.surface.values.begin());
    std::copy_n(underground, 7, counts.underground.values.begin());
    return counts;
}

} // namespace

CountWithCap<uint32_t> getSpawnableMobTickUsage() {
    const auto& spawner = getBedrockSpawner();
    return {
        static_cast<uint32_t>(spawner.mSpawnableMobTickCount),
        static_cast<uint32_t>(PopulationCapManager::getInstance().globalMax)
    };
}

CountWithCap<SpawnDensityCounts> getBaseTypeDensity(BlockSource& region, ChunkPos const& chunkPos) {
    auto& spawner = getBedrockSpawner();
    spawner._updateBaseTypeCount(region, chunkPos);
    const auto& data      = spawner.mBaseTypeCount;
    const auto  count     = toSpawnDensityCounts(data[kSpawnSurfaceIdx], data[kSpawnUndergroundIdx]);
    const auto& dimension = region.getDimension();
    const auto  caps      = toSpawnDensityCounts(dimension.mMobsPerChunkSurface, dimension.mMobsPerChunkUnderground);
    return {count, caps};
}

EntityTypeCounts getEntityTypeCounts(BlockSource& region, ChunkPos const& chunkPos) {
    auto& spawner = getBedrockSpawner();
    spawner._updateBaseTypeCount(region, chunkPos);
    const auto&      data = spawner.mEntityTypeCount;
    EntityTypeCounts counts;
    for (const auto& [type, count] : *data[kSpawnSurfaceIdx]) {
        counts.surface[type] = count;
    }
    for (const auto& [type, count] : *data[kSpawnUndergroundIdx]) {
        counts.underground[type] = count;
    }
    return counts;
}

bool isOnSurface(BlockSource const& region, BlockPos const& pos) {
    BlockPos findPos{pos.x, region.getDimension().mHeightRange->mMax, pos.z};
    bool     hasFound  = false;
    bool     isSurface = true;
    while (Spawner::findNextSpawnBlockUnder(region, findPos, std::nullopt, SpawnBlockRequirements::None)) {
        if (findPos.y == pos.y) {
            hasFound = true;
            break;
        }
        isSurface = false;
    }
    if (!hasFound) {
        throw std::runtime_error("Failed to determine if the block is on surface");
    }
    return isSurface;
}

struct LiquidBlockInfo {
    bool isWater  = false;
    bool isBubble = false;
    bool isLava   = false;

    LiquidBlockInfo(BlockSource const& region, BlockPos const& pos) {
        const auto& type = region.getMaterial(pos).mType;
        isWater          = type == ::SharedTypes::v1_26_20::MaterialType::Water;
        isBubble         = type == ::SharedTypes::v1_26_20::MaterialType::Bubble;
        isLava           = type == ::SharedTypes::v1_26_20::MaterialType::Lava;
    }
};

const SpawnConditions getSpawnConditions(BlockSource& region, BlockPos const& pos) {
    bool        isSurface = isOnSurface(region, pos);
    const auto& info1     = LiquidBlockInfo(region, utils::up(pos, 1));
    const auto& info2     = LiquidBlockInfo(region, utils::up(pos, 2));

    int rawBrightness = region.getBrightness(pos);

    return SpawnConditions{
        isSurface,
        isSurface ? info1.isWater : info1.isWater && info2.isWater,
        isSurface ? info1.isBubble : info1.isBubble && info2.isBubble,
        isSurface ? info1.isLava : info1.isLava && info2.isLava,
        !isSurface,
        region.getLevel().getCurrentTick().tickID,
        rawBrightness,
        pos
    };
}

std::vector<MobSpawnerData*>
getCandidateMobs(BlockSource& region, BlockPos const& pos, SpawnConditions const& conditions) {
    std::vector<MobSpawnerData*> result;

    const auto& biome = region.getBiome(pos);
    for (const std::shared_ptr<::MobSpawnerData>& data : biome.mMobs.get()) {
        if (!data) continue;
        if (data->mSpawnRules->canSpawnInConditions(conditions, region)) {
            result.push_back(data.get());
        }
    }
    return result;
}
} // namespace coral_fans::functions