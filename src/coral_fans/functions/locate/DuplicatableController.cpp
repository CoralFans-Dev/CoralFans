#include "DuplicatableController.h"
#include "coral_fans/Config.h"
#include "coral_fans/CoralFans.h"


#include "ll/api/service/Bedrock.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/storage/DBChunkStorage.h"


namespace coral_fans::functions::locate {

bool DuplicatableController::isChunkValid(BlockSource& region, ChunkPos originChunkPos) {
    auto* dbChunkStorage = static_cast<DBChunkStorage*>(&(*region.getDimension().mChunkSource->mOwnedParent));
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
            if (!dbChunkStorage->isChunkSaved(chunkPos)) return true;
        }
    }
    return false;
}

void DuplicatableController::tryRemoveChunkData(ChunkPos originChunkPos) {
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    auto  originIter    = this->bsciChunkData.find(originChunkPos);
    if (originIter != this->bsciChunkData.end() && originIter->second->dataDrawed) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (!i && !j) continue;
                ChunkPos chunkPos     = originChunkPos + ChunkPos(i, j);
                auto     neighborIter = this->bsciChunkData.find(chunkPos);
                if (neighborIter == this->bsciChunkData.end()) continue;
                neighborIter->second->neighborValidCount--;
                if (!neighborIter->second->neighborValidCount && !neighborIter->second->dataDrawed) {
                    geometryGroup->remove(neighborIter->second->chunkSavedDrawGeoId);
                    this->bsciChunkData.erase(neighborIter);
                }
            }
        }
        this->removeAllGeoIds(*originIter->second);
        if (!originIter->second->neighborValidCount) {
            geometryGroup->remove(originIter->second->chunkSavedDrawGeoId);
            this->bsciChunkData.erase(originIter);
        } else originIter->second->dataDrawed = 0;
    }
}

void DuplicatableController::removeInvalidData(class ChunkSource& chunkSource) {
    std::lock_guard lock(this->dataMapLock);
    std::erase_if(this->dataMap, [&chunkSource](auto&& iter) { return !chunkSource.getExistingChunk(iter.first); });
}

void DuplicatableController::removeBsciDataByType(uint showType) {
    if (!(showType & this->responsibleShowTypes)) return;

    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();

    std::vector<ChunkPos> toRemove;

    std::lock_guard lock(this->bsciChunkDataLock);
    for (auto& [originChunkPos, baseData] : this->bsciChunkData) {
        auto* geoId = this->getGeoIdByShowType(*baseData, showType);
        if (geoId && geoId->value) {
            geometryGroup->remove(*geoId);
            geoId->value          = 0;
            baseData->dataDrawed &= ~showType;
            if (!baseData->dataDrawed) {
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        if (!i && !j) continue;
                        ChunkPos chunkPos     = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                        auto     neighborIter = this->bsciChunkData.find(chunkPos);
                        if (neighborIter == this->bsciChunkData.end()) continue;
                        neighborIter->second->neighborValidCount--;
                        if (!neighborIter->second->neighborValidCount && !neighborIter->second->dataDrawed) {
                            geometryGroup->remove(neighborIter->second->chunkSavedDrawGeoId);
                            toRemove.emplace_back(chunkPos);
                        }
                    }
                }
                if (!baseData->neighborValidCount) {
                    geometryGroup->remove(baseData->chunkSavedDrawGeoId);
                    toRemove.emplace_back(originChunkPos);
                }
            }
        }
    }
    for (const auto& key : toRemove) {
        this->bsciChunkData.erase(key);
    }
}

void DuplicatableController::bsciDataRuntimeRemoveInternal(uint showType) {
    if (!showType) return;
    auto& geometryGroup      = CoralFans::getInstance().getGeometryGroup();
    auto& duplicatableConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable;

    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;

    auto dim = level->getDimension(this->getDimId()).lock();
    if (!dim) return;

    auto* dbChunkStorage = static_cast<DBChunkStorage*>(&(*dim->mChunkSource->mOwnedParent));

    std::vector<ChunkPos> toRemove;

    std::lock_guard lock(this->bsciChunkDataLock);
    for (auto& [originChunkPos, baseData] : this->bsciChunkData) {
        if (baseData->dataDrawed && baseData->runtimeRemoveTickCounter == duplicatableConfig.runtimeRemoveScale) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (!i && !j) continue;
                    ChunkPos chunkPos     = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                    auto     neighborIter = this->bsciChunkData.find(chunkPos);
                    if (neighborIter == this->bsciChunkData.end()) continue;
                    neighborIter->second->neighborValidCount--;
                    if (!neighborIter->second->neighborValidCount && !neighborIter->second->dataDrawed) {
                        geometryGroup->remove(neighborIter->second->chunkSavedDrawGeoId);
                        toRemove.emplace_back(chunkPos);
                    }
                }
            }
            this->removeAllGeoIds(*baseData);

            if (!baseData->neighborValidCount) {
                geometryGroup->remove(baseData->chunkSavedDrawGeoId);
                toRemove.emplace_back(originChunkPos);
            }
            baseData->dataDrawed = 0;
            continue;
        }
        baseData->runtimeRemoveTickCounter++;
        if (baseData->chunkSaved || (!baseData->neighborValidCount && !baseData->dataDrawed)) continue;
        if (dbChunkStorage->isChunkSaved(originChunkPos)) {
            baseData->chunkSaved = true;
            geometryGroup->remove(baseData->chunkSavedDrawGeoId);
            baseData->chunkSavedDrawGeoId = geometryGroup->box(
                this->getDimId(),
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
            );
        }
    }
    for (const auto& key : toRemove) {
        this->bsciChunkData.erase(key);
    }
}

void DuplicatableController::drawChunkSavedInfo(
    BlockSource&                                                      region,
    ChunkPos                                                          originChunkPos,
    BsciChunkDataBase&                                                originChunkData,
    std::unordered_map<ChunkPos, std::unique_ptr<BsciChunkDataBase>>& bsciChunkData
) {
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    auto* dbChunkStorage =
        reinterpret_cast<class DBChunkStorage*>(&(*region.getDimension().mChunkSource->mOwnedParent));
    auto& duplicatableConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable;
    int   dimId              = region.getDimensionId();
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) continue;
            ChunkPos chunkPos             = originChunkPos + ChunkPos(i, j);
            auto [neighborIter, inserted] = bsciChunkData.try_emplace(chunkPos, std::make_unique<BsciChunkDataBase>());
            if (inserted) {
                if (!dbChunkStorage->isChunkSaved(chunkPos)) {
                    neighborIter->second->chunkSaved          = false;
                    neighborIter->second->chunkSavedDrawGeoId = geometryGroup->box(
                        dimId,
                        {Vec3(chunkPos.x * 16 + 0.1, 0, chunkPos.z * 16 + 0.1),
                         Vec3(chunkPos.x * 16 + 15.9, 128, chunkPos.z * 16 + 15.9)},
                        mce::Color(duplicatableConfig.chunkSavedDebugInfo.unsavedChunk)
                    );
                } else {
                    neighborIter->second->chunkSaved          = true;
                    neighborIter->second->chunkSavedDrawGeoId = geometryGroup->box(
                        dimId,
                        {Vec3(chunkPos.x * 16 + 0.1, 0, chunkPos.z * 16 + 0.1),
                         Vec3(chunkPos.x * 16 + 15.9, 128, chunkPos.z * 16 + 15.9)},
                        mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
                    );
                }
            }
            neighborIter->second->neighborValidCount++;
        }
    }
    if (!originChunkData.chunkSavedDrawGeoId.value) {
        if (!dbChunkStorage->isChunkSaved(originChunkPos)) {
            originChunkData.chunkSaved          = false;
            originChunkData.chunkSavedDrawGeoId = geometryGroup->box(
                dimId,
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.unsavedChunk)
            );
        } else {
            originChunkData.chunkSaved          = true;
            originChunkData.chunkSavedDrawGeoId = geometryGroup->box(
                dimId,
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
            );
        }
    }
}

void DuplicatableController::clearInternal() {
    {
        std::lock_guard lock(this->decorationThreadIdsLock);
        this->decorationThreadIds.clear();
    }
    {
        std::lock_guard lock(this->dataMapLock);
        this->dataMap.clear();
    }
    {
        std::lock_guard lock(this->bsciChunkDataLock);
        auto&           geometryGroup = CoralFans::getInstance().getGeometryGroup();
        for (auto& [_, baseChunkData] : this->bsciChunkData) {
            this->removeAllGeoIds(*baseChunkData);
            if (baseChunkData->chunkSavedDrawGeoId.value) geometryGroup->remove(baseChunkData->chunkSavedDrawGeoId);
        }
        this->bsciChunkData.clear();
    }
}

} // namespace coral_fans::functions::locate
