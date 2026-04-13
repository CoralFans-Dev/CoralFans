#include "../popcap/PopCapManager.h"
#include "MineruleManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/MobSpawnRules.h"
#include "mc/world/level/biome/MobSpawnerData.h"
#include "mc/world/level/biome/SpawnConditions.h"
#include "mc/world/level/dimension/Dimension.h"


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    handlePopCapHook,
    HookPriority::Normal,
    BedrockSpawner,
    &BedrockSpawner::_handlePopulationCap,
    int,
    MobSpawnerData const*  mobType,
    SpawnConditions const& conditions,
    int                    inSpawnCount
) {
    if (!PopulationCapManager::getInstance().enabled) {
        return origin(mobType, conditions, inSpawnCount);
    }

    // global
    int maxCount = PopulationCapManager::getInstance().globalMax;
    if (maxCount >= 0 && this->mTotalEntityCount + inSpawnCount > maxCount) {
        inSpawnCount = std::max(0, maxCount - this->mTotalEntityCount);
    }

    // single type (keep vanilla)
    int populationCap =
        conditions.isOnSurface ? mobType->mSpawnRules->mSurfaceCap : mobType->mSpawnRules->mUndergroundCap;
    if (populationCap >= 0) {
        auto realType    = mobType->mIdentifier->mCanonicalName.get();
        int  isOnSurface = conditions.isOnSurface ? 1 : 0;

        auto map             = this->mEntityTypeCount[isOnSurface].get();
        auto it              = map.find(realType);
        int  entityTypeCount = (it != map.end()) ? it->second : 0;

        if (entityTypeCount + inSpawnCount > populationCap) {
            inSpawnCount = std::max(0, populationCap - entityTypeCount);
        }
    }

    return inSpawnCount;
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    DimInitHook,
    HookPriority::Normal,
    Dimension,
    &Dimension::$init,
    void,
    const br::worldgen::StructureSetRegistry& structureSetRegistry
) {
    origin(structureSetRegistry);

    int   dimId   = static_cast<int>(this->getDimensionId().id);
    auto& manager = PopulationCapManager::getInstance();

    if (manager.backupCaps.find(dimId) == manager.backupCaps.end()) {
        mDimension original(dimId);
        std::copy_n(std::begin(this->mMobsPerChunkSurface), 7, original.surfaceCaps.begin());
        std::copy_n(std::begin(this->mMobsPerChunkUnderground), 7, original.undergroundCaps.begin());
        manager.backupCaps.emplace(dimId, original);
    }

    if (manager.enabled) {
        auto it = manager.currentCaps.find(dimId);
        if (it != manager.currentCaps.end()) {
            std::copy_n(it->second.surfaceCaps.begin(), 7, std::begin(this->mMobsPerChunkSurface));
            std::copy_n(it->second.undergroundCaps.begin(), 7, std::begin(this->mMobsPerChunkUnderground));
        }
    }
}

void MineruleManager::populationCapHook(bool bl) {
    bl ? handlePopCapHook::hook() : handlePopCapHook::unhook();
    PopulationCapManager::getInstance().setEnabled(bl);
}

} // namespace coral_fans::functions