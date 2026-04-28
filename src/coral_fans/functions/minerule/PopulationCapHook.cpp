#include "MineruleManager.h"
#include "coral_fans/CoralFans.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/biome/MobSpawnRules.h"
#include "mc/world/level/biome/MobSpawnerData.h"
#include "mc/world/level/biome/SpawnConditions.h"
#include "mc/world/level/dimension/Dimension.h"


#ifdef LL_PLAT_C
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


#include <algorithm>
#include <array>
#include <memory>
#include <string>


namespace coral_fans::functions {
void PopulationCapManager::init() {
    auto& db      = CoralFans::getInstance().getConfigDb();
    auto& manager = PopulationCapManager::getInstance();
    if (auto val = db->get("populationCap.enabled")) {
        manager.enabled = val.value() == "true";
    }
    if (auto val = db->get("populationCap.globalMax")) {
        manager.globalMax = std::stoi(val.value());
    }

    for (int dimId = 0; dimId <= 2; dimId++) {
        std::string key = "populationCap.dim" + std::to_string(dimId);
        if (auto val = db->get(key)) {
            manager.currentCaps[dimId] = std::make_unique<DimensionData>(val.value());
        }
    }
}

void PopulationCapManager::setEnabled(bool bl) {
    enabled = bl;
    CoralFans::getInstance().getConfigDb()->set("populationCap.enabled", bl ? "true" : "false");
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;
    if (!bl) {
        // restore dimension caps
        for (int dimId = 0; dimId < 3; dimId++) {
            if (backupCaps[dimId]) {
                if (auto dim = level->getDimension(dimId).lock()) {
                    std::copy_n(backupCaps[dimId]->surfaceCaps.begin(), 7, std::begin(dim->mMobsPerChunkSurface));
                    std::copy_n(
                        backupCaps[dimId]->undergroundCaps.begin(),
                        7,
                        std::begin(dim->mMobsPerChunkUnderground)
                    );
                }
            }
        }

    } else {
        // apply current dimension caps
        for (int dimId = 0; dimId < 3; dimId++) {
            if (currentCaps[dimId]) {
                if (auto dim = level->getDimension(dimId).lock()) {
                    std::copy_n(currentCaps[dimId]->surfaceCaps.begin(), 7, dim->mMobsPerChunkSurface);
                    std::copy_n(currentCaps[dimId]->undergroundCaps.begin(), 7, dim->mMobsPerChunkUnderground);
                }
            }
        }
    }
}

void PopulationCapManager::setGlobalMax(int count) {
    globalMax = count;
    CoralFans::getInstance().getConfigDb()->set("populationCap.globalMax", std::to_string(count));
}

bool PopulationCapManager::setDimCap(int dimId, int category, bool isOnSurface, float count) {
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return false;
    auto dim = level->getDimension(dimId).lock();
    if (!dim) return false;

    if (!backupCaps[dimId]) {
        backupCaps[dimId] = std::make_unique<DimensionData>();
        std::copy_n(std::begin(dim->mMobsPerChunkSurface), 7, backupCaps[dimId]->surfaceCaps.begin());
        std::copy_n(std::begin(dim->mMobsPerChunkUnderground), 7, backupCaps[dimId]->undergroundCaps.begin());
    }

    if (!currentCaps[dimId]) {
        currentCaps[dimId] = std::make_unique<DimensionData>();
        std::copy_n(std::begin(dim->mMobsPerChunkSurface), 7, currentCaps[dimId]->surfaceCaps.begin());
        std::copy_n(std::begin(dim->mMobsPerChunkUnderground), 7, currentCaps[dimId]->undergroundCaps.begin());
    }

    if (isOnSurface) currentCaps[dimId]->surfaceCaps[category] = count;
    else currentCaps[dimId]->undergroundCaps[category] = count;


    if (enabled) {
        if (isOnSurface) dim->mMobsPerChunkSurface[category] = count;
        else dim->mMobsPerChunkUnderground[category] = count;
    }

    CoralFans::getInstance().getConfigDb()->set(
        "populationCap.dim" + std::to_string(dimId),
        currentCaps[dimId]->toBytes()
    );

    return true;
}

bool PopulationCapManager::resetDimCap(int dimId) {
    auto dimLock = ll::service::getLevel()->getDimension(dimId).lock();
    if (!dimLock) return false;

    if (!backupCaps[dimId]) return false;

    std::copy_n(backupCaps[dimId]->surfaceCaps.begin(), 7, dimLock->mMobsPerChunkSurface);
    std::copy_n(backupCaps[dimId]->undergroundCaps.begin(), 7, dimLock->mMobsPerChunkUnderground);

    currentCaps[dimId] = std::make_unique<DimensionData>(*backupCaps[dimId]);

    CoralFans::getInstance().getConfigDb()->set(
        "populationCap.dim" + std::to_string(dimId),
        backupCaps[dimId]->toBytes()
    );
    return true;
}

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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(mobType, conditions, inSpawnCount);
#endif
    auto& popCapManager = PopulationCapManager::getInstance();

    if (!popCapManager.enabled) {
        return origin(mobType, conditions, inSpawnCount);
    }

    // global
    int maxCount = popCapManager.globalMax;
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(structureSetRegistry);
#endif
    origin(structureSetRegistry);

    int   dimId   = this->getDimensionId();
    auto& manager = PopulationCapManager::getInstance();

    if (!manager.backupCaps[dimId]) {
        manager.backupCaps[dimId] = std::make_unique<PopulationCapManager::DimensionData>();
        std::copy_n(std::begin(this->mMobsPerChunkSurface), 7, manager.backupCaps[dimId]->surfaceCaps.begin());
        std::copy_n(std::begin(this->mMobsPerChunkUnderground), 7, manager.backupCaps[dimId]->undergroundCaps.begin());
    }

    if (manager.enabled) {
        if (manager.currentCaps[dimId]) {
            std::copy_n(manager.currentCaps[dimId]->surfaceCaps.begin(), 7, std::begin(this->mMobsPerChunkSurface));
            std::copy_n(
                manager.currentCaps[dimId]->undergroundCaps.begin(),
                7,
                std::begin(this->mMobsPerChunkUnderground)
            );
        }
    }
}

void populationCapHook(bool bl) {
    if (bl) {
        handlePopCapHook::hook();
        DimInitHook::hook();
    } else {
        handlePopCapHook::unhook();
        DimInitHook::unhook();
    }
    PopulationCapManager::getInstance().setEnabled(bl);
}

void PopulationCapManager::clear() {
    for (auto& item : this->backupCaps) item = nullptr;
    for (auto& item : this->currentCaps) item = nullptr;
}
} // namespace coral_fans::functions