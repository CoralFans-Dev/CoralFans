#include "PopCapManager.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"
#include <algorithm>
#include <array>
#include <string>


namespace coral_fans::functions {

void PopulationCapManager::init() {
    auto& db      = coral_fans::mod().getConfigDb();
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
            manager.currentCaps.emplace(dimId, mDimension(val.value()));
        }
    }
}

void PopulationCapManager::setEnabled(bool bl) {
    enabled = bl;
    coral_fans::mod().getConfigDb()->set("populationCap.enabled", bl ? "true" : "false");

    if (!bl) {
        // restore dimension caps
        if (auto level = ll::service::getLevel()) {
            for (auto& [dimId, dimData] : backupCaps) {
                if (auto dimLock = level->getDimension(dimId).lock()) {
                    std::copy_n(dimData.surfaceCaps.begin(), 7, dimLock->mMobsPerChunkSurface);
                    std::copy_n(dimData.undergroundCaps.begin(), 7, dimLock->mMobsPerChunkUnderground);
                }
            }
        }
    } else {
        // apply current dimension caps
        if (auto level = ll::service::getLevel()) {
            for (auto& [dimId, dimData] : currentCaps) {
                if (auto dimLock = level->getDimension(dimId).lock()) {
                    std::copy_n(dimData.surfaceCaps.begin(), 7, dimLock->mMobsPerChunkSurface);
                    std::copy_n(dimData.undergroundCaps.begin(), 7, dimLock->mMobsPerChunkUnderground);
                }
            }
        }
    }
}

void PopulationCapManager::setGlobalMax(int count) {
    globalMax = count;
    coral_fans::mod().getConfigDb()->set("populationCap.globalMax", std::to_string(count));
}

bool PopulationCapManager::setDimCap(int dimId, int category, bool isOnSurface, float count) {
    auto dimLock = ll::service::getLevel()->getDimension(dimId).lock();
    if (!dimLock) return false;

    if (backupCaps.find(dimId) == backupCaps.end()) {
        mDimension original(dimId);
        std::copy_n(std::begin(dimLock->mMobsPerChunkSurface), 7, original.surfaceCaps.begin());
        std::copy_n(std::begin(dimLock->mMobsPerChunkUnderground), 7, original.undergroundCaps.begin());
        backupCaps.emplace(dimId, original);

        if (currentCaps.find(dimId) == currentCaps.end()) {
            currentCaps.emplace(dimId, original);
        }
    }

    if (isOnSurface) {
        currentCaps[dimId].surfaceCaps[category] = count;
    } else {
        currentCaps[dimId].undergroundCaps[category] = count;
    }

    if (enabled) {
        if (isOnSurface) {
            dimLock->mMobsPerChunkSurface[category] = count;
        } else {
            dimLock->mMobsPerChunkUnderground[category] = count;
        }
    }

    coral_fans::mod().getConfigDb()->set("populationCap.dim" + std::to_string(dimId), currentCaps[dimId].toBytes());

    return true;
}

bool PopulationCapManager::resetDimCap(int dimId) {
    auto dimLock = ll::service::getLevel()->getDimension(dimId).lock();
    if (!dimLock) return false;

    auto it = backupCaps.find(dimId);
    if (it == backupCaps.end()) return false;

    const mDimension& original = it->second;
    std::copy_n(original.surfaceCaps.begin(), 7, dimLock->mMobsPerChunkSurface);
    std::copy_n(original.undergroundCaps.begin(), 7, dimLock->mMobsPerChunkUnderground);

    currentCaps[dimId] = original;

    coral_fans::mod().getConfigDb()->set("populationCap.dim" + std::to_string(dimId), original.toBytes());
    return true;
}
} // namespace coral_fans::functions