#pragma once

#include "mc/deps/core/string/HashedString.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

class BlockPos;
class BlockSource;
class ChunkPos;
class MobSpawnerData;
class SpawnConditions;

namespace coral_fans::functions {

template <typename T>
struct CountWithCap {
    T count;
    T cap;
};

// animal, monster, water_animal, villager, ambient, cat, pillager
struct MobCategoryCounts {
    std::array<int, 7> values{};
};

struct SpawnDensityCounts {
    MobCategoryCounts surface;
    MobCategoryCounts underground;
};

struct EntityTypeCounts {
    std::unordered_map<::HashedString, int> surface;
    std::unordered_map<::HashedString, int> underground;
};

CountWithCap<uint32_t> getSpawnableMobTickUsage();

CountWithCap<SpawnDensityCounts> getBaseTypeDensity(BlockSource& region, ChunkPos const& chunkPos);

EntityTypeCounts getEntityTypeCounts(BlockSource& region, ChunkPos const& chunkPos);

const SpawnConditions getSpawnConditions(BlockSource& region, BlockPos const& pos);

std::vector<MobSpawnerData*>
getCandidateMobs(BlockSource& region, BlockPos const& pos, SpawnConditions const& conditions);
} // namespace coral_fans::functions