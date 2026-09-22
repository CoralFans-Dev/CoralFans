#include "coral_fans/functions/spawn/Spawn.h"

#include "mc/util/Random.h"
#include "mc/deps/game_refs/GameRefs.h"
#include "mc/deps/ecs/gamerefs_entity/GameRefsEntity.h"

#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/minerule/MineruleManager.h"

#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorSpawnRuleGroup.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/biome/MobSpawnHerdInfo.h"
#include "mc/world/level/biome/MobSpawnerData.h"
#include "mc/world/level/biome/MobSpawnerPermutation.h"
#include "mc/world/level/biome/SpawnConditions.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockType.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"


#include <algorithm>
#include <cmath>
#include <cstdlib>
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

ActorTypeCounts countActors(Player const& player, SpawnCountScope scope) {
    const auto centerChunk  = utils::blockPosToChunkPos(player.getFeetBlockPos());
    const auto dimensionId  = player.getDimensionId();
    ActorTypeCounts counts;

    for (const auto& entity : player.getLevel().getEntities()) {
        if (!entity) continue;
        const auto* actor = entity.tryUnwrap<Actor>().as_ptr();
        if (!actor || actor->getDimensionId() != dimensionId) continue;

        if (scope != SpawnCountScope::All) {
            const auto actorChunk = utils::blockPosToChunkPos(actor->getFeetBlockPos());
            if (scope == SpawnCountScope::Chunk) {
                if (actorChunk.x != centerChunk.x || actorChunk.z != centerChunk.z) continue;
            } else if (
                std::abs(actorChunk.x - centerChunk.x) > 4 || std::abs(actorChunk.z - centerChunk.z) > 4
            ) {
                continue;
            }
        }
        ++counts[actor->getTypeName()];
    }
    return counts;
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
        0,
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

std::vector<std::string> spawnCluster(BlockSource& region, BlockPos pos) {
    // cluster spawning needs a non-air ground block; walk down to the next
    // spawnable block like natural spawning does
    if (region.getBlock(pos).isAir()
        && !Spawner::findNextSpawnBlockUnder(region, pos, std::nullopt, SpawnBlockRequirements::None)) {
        throw std::runtime_error("Failed to find a spawnable block under the position");
    }

    auto& spawner = getBedrockSpawner();
    // BedrockSpawner::tick refreshes the density counters before spawning a
    // cluster; without this the cap checks below would read stale counts
    spawner._updateBaseTypeCount(region, utils::blockPosToChunkPos(pos));

    auto conditions          = getSpawnConditions(region, pos);
    conditions.rawBrightness = region.getBrightness(utils::up(pos, 1));

    // same mob selection as BedrockSpawner::_spawnMobCluster: the ground
    // block picks a mob from the biome spawn list with the spawn conditions
    auto mobConditions = conditions;
    if (!conditions.isInWater && !conditions.isInLava) mobConditions.pos = utils::up(pos, 1);
    const auto* mobData = region.getBlock(pos).getBlockType().getMobToSpawn(mobConditions, region);
    if (!mobData) return {};

    const auto& rules = *mobData->mSpawnRules;
    if (!rules.canSpawnInConditions(conditions, region)) return {};

    auto&       random = region.getLevel().getRandom();
    const auto& herd   = rules.selectRandomHerd(random);
    const float roll   = random.nextFloat();
    int         count  = static_cast<int>(herd.mMinCount)
              + static_cast<int>(std::lround(roll * roll * static_cast<float>(herd.mMaxCount - herd.mMinCount)));
    if (count <= 0) return {};

    // _spawnMobCluster calls isSpawnPositionOk with a4 = true here, which
    // additionally rejects positions within mMinSpawnDistance (24 by default)
    // of any player and positions in non-ticking chunks; an explicit command
    // target must not depend on that, so only the block/fluid checks are kept
    if (!Spawner::isSpawnPositionOk(rules, region, utils::up(pos, 1), false)) return {};

    // global spawnable mob cap, same formula as _spawnMobCluster
    const int ticked    = static_cast<int>(static_cast<uint>(spawner.mSpawnableMobTickCount));
    const int globalCap = PopulationCapManager::getInstance().globalMax;
    if (count + ticked > globalCap) count = std::max(globalCap - ticked, 0);

    // per-type cap from the mob's spawn rules
    const int typeCap =
        conditions.isOnSurface ? static_cast<int>(rules.mSurfaceCap) : static_cast<int>(rules.mUndergroundCap);
    if (typeCap >= 0) {
        const auto& typeCounts =
            *spawner.mEntityTypeCount[conditions.isOnSurface ? kSpawnSurfaceIdx : kSpawnUndergroundIdx];
        int current = 0;
        if (const auto it = typeCounts.find(mobData->mIdentifier->mCanonicalName); it != typeCounts.end())
            current = it->second;
        if (current + count >= typeCap) count = typeCap - current;
    }

    std::vector<Mob*> spawnGroup;
    auto&             dimension = region.getDimension();
    const size_t      surfIdx   = conditions.isOnSurface ? kSpawnSurfaceIdx : kSpawnUndergroundIdx;

    // per-category density cap, checked per spawn like _spawnMobCluster;
    // a blocked spawn still consumes from the rolled count, as vanilla does
    auto trySpawn = [&](::ActorDefinitionIdentifier const& id) {
        const int pool = spawner.mActorSpawnRules->getActorSpawnPool(id);
        if (pool >= 0) {
            const int poolCap = static_cast<int>(
                conditions.isOnSurface ? dimension.mMobsPerChunkSurface[pool]
                                       : dimension.mMobsPerChunkUnderground[pool]
            );
            if (poolCap <= 0 || spawner.mBaseTypeCount[surfIdx][pool] >= poolCap) return;
        }
        spawner._spawnMobInCluster(region, id, pos, conditions, spawnGroup);
    };

    // guaranteed permutations (spawn rules with "guaranteed_count") spawn first
    const auto& guaranteedList =
        static_cast<const std::vector<::MobSpawnerPermutation>&>(rules.mGuaranteedList);
    for (const auto& permutation : guaranteedList) {
        for (int i = 0; i < permutation.mRandomWeight && count-- > 0; ++i) trySpawn(permutation.mId);
    }

    // remaining spawns pick a weighted random permutation, like vanilla
    const auto& permutationList =
        static_cast<const std::vector<::MobSpawnerPermutation>&>(rules.mPermutationList);
    while (count-- > 0) {
        const auto* id = &static_cast<const ::ActorDefinitionIdentifier&>(*mobData->mIdentifier);
        int       totalWeight = 0;
        for (const auto& permutation : permutationList) totalWeight += permutation.mRandomWeight;
        if (totalWeight > 0) {
            int rollWeight = random.nextInt(totalWeight);
            for (const auto& permutation : permutationList) {
                rollWeight -= permutation.mRandomWeight;
                if (rollWeight < 0) {
                    id = &static_cast<const ::ActorDefinitionIdentifier&>(permutation.mId);
                    break;
                }
            }
        }
        trySpawn(*id);
    }
    if (!spawnGroup.empty()) BedrockSpawner::_sendHerdEvents(herd, spawnGroup);

    std::vector<std::string> spawned;
    spawned.reserve(spawnGroup.size());
    for (const auto* mob : spawnGroup) spawned.push_back(mob->getTypeName());
    return spawned;
}
} // namespace coral_fans::functions