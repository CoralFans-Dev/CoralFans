#include "DuplicatableManager.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/chunk/ChunkState.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/feature/NoSurfaceOreFeature.h"
#include "mc/world/level/levelgen/v1/NetherGenerator.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>


namespace coral_fans::functions::locate {
LL_TYPE_INSTANCE_HOOK(
    DuplicatableManager::DuplicatableHook1,
    ll::memory::HookPriority::Normal,
    NetherGenerator,
    &NetherGenerator ::$decorationPostProcessChunk,
    bool,
    ChunkViewSource& neighborhood
) {
    auto            dim            = neighborhood.mDimension;
    DBChunkStorage* dbChunkStorage = static_cast<DBChunkStorage*>(&(*dim->mChunkSource->mOwnedParent));
    auto&           pos            = neighborhood.mArea->mBounds.mMin;
    ChunkPos        originChunkPos = ChunkPos(pos->x + 1, pos->z + 1);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
            if (!dbChunkStorage->isChunkSaved(chunkPos)) {
                auto& DuplicatableManager = DuplicatableManager::getInstance();
                auto  threadId            = std::this_thread::get_id();
                {
                    std::lock_guard lock(DuplicatableManager.netherDecorationThreadIdsLock);
                    DuplicatableManager.netherDecorationThreadIds.push_back(threadId);
                }
                auto ori = origin(neighborhood);
                {
                    std::lock_guard lock(DuplicatableManager.netherDecorationThreadIdsLock);
                    std::erase_if(DuplicatableManager.netherDecorationThreadIds, [threadId](const std::thread::id& id) {
                        return id == threadId;
                    });
                }
                return ori;
            }
        }
    }
    return origin(neighborhood);
}

LL_TYPE_INSTANCE_HOOK(
    DuplicatableManager::DuplicatableHook2,
    ll::memory::HookPriority::Normal,
    NoSurfaceOreFeature,
    &NoSurfaceOreFeature ::$place,
    std::optional<::BlockPos>,
    IFeature::PlacementContext const& context
) {
    auto ori = origin(context);
    if (ori.has_value()) {
        auto  threadId            = std::this_thread::get_id();
        auto& DuplicatableManager = DuplicatableManager::getInstance();
        {
            std::lock_guard lock(DuplicatableManager.netherDecorationThreadIdsLock);
            if (std::find(
                    DuplicatableManager.netherDecorationThreadIds.begin(),
                    DuplicatableManager.netherDecorationThreadIds.end(),
                    threadId
                )
                == DuplicatableManager.netherDecorationThreadIds.end()) {
                return ori;
            }
        }
        ChunkPos chunkPos = ChunkPos(*context.mPos);
        if (ChunkPos(ori.value()) != chunkPos) {
            std::lock_guard lock(DuplicatableManager.netherDataMapLock);
            auto [iter, _] = DuplicatableManager.netherDataMap.try_emplace(chunkPos);
            iter->second.ancientDebrisPosSet.emplace(std::make_pair(*context.mPos, ori.value()));
        }
    }
    return ori;
}

void DuplicatableManager::removeData() {
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;
    if (auto netherDim = level->getDimension(1).lock()) {
        auto& netherMainBlockSource = netherDim->mChunkSource;
        {
            std::lock_guard lock(this->netherDataMapLock);
            std::erase_if(this->netherDataMap, [&netherMainBlockSource](auto&& iter) {
                return !netherMainBlockSource->getExistingChunk(iter.first);
            });
        }
    }
}

bsci::GeometryGroup::GeoId
DuplicatableManager::drawAncientDebris(std::unordered_set<std::pair<BlockPos, BlockPos>, BlockPosPairHash>& data) {
    using ll::i18n_literals::operator""_tr;
    auto&                                   geometryGroup      = coral_fans::mod().getGeometryGroup();
    auto&                                   duplicatableConfig = coral_fans::mod().getConfig().locate.duplicatable;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(4 * data.size());
    for (auto& [oriPos, endPos] : data) {
        geoIdList.emplace_back(geometryGroup->box(
            1,
            AABB(oriPos, oriPos + BlockPos(1, 1, 1)),
            mce::Color(duplicatableConfig.ancientDebris.originPosColor)
        ));
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
            "translate.locate.duplicatable.ancientDebris.oriPos"_tr()
        ));
        geoIdList.emplace_back(geometryGroup->box(
            1,
            AABB(endPos, endPos + BlockPos(1, 1, 1)),
            mce::Color(duplicatableConfig.ancientDebris.endPosColor)
        ));
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {endPos.x + 0.5f, endPos.y + 0.5f, endPos.z + 0.5f},
            "translate.locate.duplicatable.ancientDebris.endPos"_tr()
        ));
    }
    return geometryGroup->merge(geoIdList);
}

bool DuplicatableManager::isChunkValid(BlockSource& region, ChunkPos originChunkPos) {
    DBChunkStorage* dbChunkStorage = static_cast<DBChunkStorage*>(&(*region.getDimension().mChunkSource->mOwnedParent));
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
            auto     chunk    = region.getChunk(chunkPos);
            // if ((!chunk || *chunk->mLoadState != ChunkState::Loaded || chunk->isNonActorDataDirty())
            // && !dbChunkStorage->isChunkSaved(chunkPos))
            // return true;
            if (!dbChunkStorage->isChunkSaved(chunkPos)) return true;
        }
    }
    return false;
}

void DuplicatableManager::removeNetherChunkData(ChunkPos originChunkPos) {
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    auto  originIter    = this->netherBsciChunkData.find(originChunkPos);
    if (originIter != this->netherBsciChunkData.end() && originIter->second.dataDrawed) {
        if (originIter->second.ancientDebrisGeoId.value) geometryGroup->remove(originIter->second.ancientDebrisGeoId);
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (!i && !j) continue;
                ChunkPos chunkPos     = originChunkPos + ChunkPos(i, j);
                auto     neighborIter = this->netherBsciChunkData.find(chunkPos);
                if (neighborIter == this->netherBsciChunkData.end()) continue;
                neighborIter->second.neighborValidCount--;
                if (!neighborIter->second.neighborValidCount && !neighborIter->second.dataDrawed) {
                    geometryGroup->remove(neighborIter->second.chunkSavedDrawGeoId);
                    this->netherBsciChunkData.erase(neighborIter);
                }
            }
        }
        if (originIter->second.ancientDebrisGeoId.value) geometryGroup->remove(originIter->second.ancientDebrisGeoId);
        if (!originIter->second.neighborValidCount) {
            geometryGroup->remove(originIter->second.chunkSavedDrawGeoId);
            this->netherBsciChunkData.erase(originIter);
        } else originIter->second.dataDrawed = false;
    }
}

void DuplicatableManager::drawChunkSavedInfo(
    BlockSource&         region,
    ChunkPos             originChunkPos,
    NetherBsciChunkData& originChunkData
) {
    auto&           geometryGroup  = coral_fans::mod().getGeometryGroup();
    DBChunkStorage* dbChunkStorage = static_cast<DBChunkStorage*>(&(*region.getDimension().mChunkSource->mOwnedParent));
    auto&           duplicatableConfig = coral_fans::mod().getConfig().locate.duplicatable;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) continue;
            ChunkPos chunkPos             = originChunkPos + ChunkPos(i, j);
            auto [neighborIter, inserted] = this->netherBsciChunkData.try_emplace(chunkPos);
            if (inserted) {
                // auto chunk = region.getChunk(chunkPos);
                // if ((!chunk || *chunk->mLoadState != ChunkState::Loaded || chunk->isNonActorDataDirty())
                // && !dbChunkStorage->isChunkSaved(chunkPos)) {
                if (!dbChunkStorage->isChunkSaved(chunkPos)) {
                    neighborIter->second.chunkSaved = false;
                    if (neighborIter->second.chunkSavedDrawGeoId.value) {
                        mod().getLogger().warn(
                            "Chunk {} load state changed but still has debug geometry, something may be wrong, pos1",
                            chunkPos.toString()
                        );
                    }
                    neighborIter->second.chunkSavedDrawGeoId = geometryGroup->box(
                        region.getDimensionId(),
                        {Vec3(chunkPos.x * 16 + 0.1, 0, chunkPos.z * 16 + 0.1),
                         Vec3(chunkPos.x * 16 + 15.9, 128, chunkPos.z * 16 + 15.9)},
                        mce::Color(duplicatableConfig.chunkSavedDebugInfo.unsavedChunk)
                    );
                } else {
                    neighborIter->second.chunkSaved = true;
                    if (neighborIter->second.chunkSavedDrawGeoId.value) {
                        mod().getLogger().warn(
                            "Chunk {} load state changed but still has debug geometry, something may be wrong, pos2",
                            chunkPos.toString()
                        );
                    }
                    neighborIter->second.chunkSavedDrawGeoId = geometryGroup->box(
                        region.getDimensionId(),
                        {Vec3(chunkPos.x * 16 + 0.1, 0, chunkPos.z * 16 + 0.1),
                         Vec3(chunkPos.x * 16 + 15.9, 128, chunkPos.z * 16 + 15.9)},
                        mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
                    );
                }
            }
            neighborIter->second.neighborValidCount++;
        }
    }
    if (!originChunkData.chunkSavedDrawGeoId.value) {
        // auto chunk = region.getChunk(originChunkPos);
        // if ((!chunk || *chunk->mLoadState != ChunkState::Loaded || chunk->isNonActorDataDirty())
        // && !dbChunkStorage->isChunkSaved(originChunkPos)) {
        if (!dbChunkStorage->isChunkSaved(originChunkPos)) {
            originChunkData.chunkSaved = false;
            if (originChunkData.chunkSavedDrawGeoId.value) {
                mod().getLogger().warn(
                    "Chunk {} load state changed but still has debug geometry, something may be wrong, pos3",
                    originChunkPos.toString()
                );
            }
            originChunkData.chunkSavedDrawGeoId = geometryGroup->box(
                region.getDimensionId(),
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.unsavedChunk)
            );
        } else {
            originChunkData.chunkSaved = true;
            if (originChunkData.chunkSavedDrawGeoId.value) {
                mod().getLogger().warn(
                    "Chunk {} load state changed but still has debug geometry, something may be wrong, pos4",
                    originChunkPos.toString()
                );
            }
            originChunkData.chunkSavedDrawGeoId = geometryGroup->box(
                region.getDimensionId(),
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
            );
        }
    }
}

void DuplicatableManager::draw() {
    if (!static_cast<int>(this->showType)) return;
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;
    level->forEachPlayer([this](Player& player) {
        int   dimId  = player.getDimensionId();
        auto& region = player.getDimensionBlockSource();
        if (dimId == 1) {
            ChunkPos originChunkPos = ChunkPos(player.getFeetBlockPos());
            for (int i = -6; i <= 6; ++i) {
                int maxJ = 6 - abs(i);
                for (int j = -maxJ; j <= maxJ; ++j) {
                    ChunkPos        chunkPos = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                    std::lock_guard lock(this->netherDataMapLock);
                    auto            netherDataMapIter = this->netherDataMap.find(chunkPos);
                    if (netherDataMapIter == this->netherDataMap.end()
                        || !(
                            this->showType & static_cast<uint>(ShowType::AncientDebris)
                            && netherDataMapIter->second.ancientDebrisPosSet.size()
                        ))
                        continue;
                    if (!this->isChunkValid(region, chunkPos)) {
                        this->netherDataMap.erase(netherDataMapIter);
                        this->removeNetherChunkData(chunkPos);
                        mod().getLogger().info("remove from {}", chunkPos.toString());
                        continue;
                    }
                    auto [netherBsciChunkDataIter, inserted] = this->netherBsciChunkData.try_emplace(chunkPos);
                    netherBsciChunkDataIter->second.runtimeRemoveTickCounter = 0;
                    if (this->showType & static_cast<uint>(ShowType::AncientDebris)
                        && netherDataMapIter->second.ancientDebrisPosSet.size()
                        && !netherBsciChunkDataIter->second.ancientDebrisGeoId.value) {
                        netherBsciChunkDataIter->second.ancientDebrisGeoId =
                            this->drawAncientDebris(netherDataMapIter->second.ancientDebrisPosSet);
                    }
                    if (!netherBsciChunkDataIter->second.dataDrawed) {
                        this->drawChunkSavedInfo(region, chunkPos, netherBsciChunkDataIter->second);
                        netherBsciChunkDataIter->second.dataDrawed = true;
                    }
                }
            }
        }
        return true;
    });
}

void DuplicatableManager::tick() {
    auto& duplicatableConfig = mod().getConfig().locate.duplicatable;
    if (!this->tickCounter) {
        this->draw();
        this->bsciDataRuntimeRemove();
        if (!this->cacheDataRemoveTickCounter) {
            this->removeData();
        }
        this->cacheDataRemoveTickCounter =
            (this->cacheDataRemoveTickCounter + 1) % duplicatableConfig.cacheDataRemoveScale;
    }
    this->tickCounter = (this->tickCounter + 1) % duplicatableConfig.drawInterval;
}

void DuplicatableManager::removeBsciData(ShowType _showType) {
    auto& geometryGroup  = coral_fans::mod().getGeometryGroup();
    this->showType      &= ~static_cast<uint>(_showType);
    if (!this->showType) {
        for (auto& [_, chunkData] : this->netherBsciChunkData) {
            if (chunkData.ancientDebrisGeoId.value) geometryGroup->remove(chunkData.ancientDebrisGeoId);
            if (chunkData.chunkSavedDrawGeoId.value) geometryGroup->remove(chunkData.chunkSavedDrawGeoId);
        }
        this->netherBsciChunkData.clear();
    }
    switch (_showType) {
    case ShowType::AncientDebris:
        std::erase_if(this->netherBsciChunkData, [&geometryGroup](auto& data) {
            if (data.second.ancientDebrisGeoId.value) {
                geometryGroup->remove(data.second.ancientDebrisGeoId);
                data.second.ancientDebrisGeoId.value = 0;
            }
            return true;
        });
    }
}

void DuplicatableManager::bsciDataRuntimeRemove() {
    if (!static_cast<int>(this->showType)) return;
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;
    auto netherDim = level->getDimension(1).lock();
    if (!netherDim) [[unlikely]]
        return;
    DBChunkStorage*       dbChunkStorage     = static_cast<DBChunkStorage*>(&(*netherDim->mChunkSource->mOwnedParent));
    auto&                 geometryGroup      = coral_fans::mod().getGeometryGroup();
    auto&                 duplicatableConfig = mod().getConfig().locate.duplicatable;
    std::vector<ChunkPos> toRemove;
    for (auto& [originChunkPos, data] : this->netherBsciChunkData) {
        if (data.dataDrawed && data.runtimeRemoveTickCounter == duplicatableConfig.runtimeRemoveScale) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (!i && !j) continue;
                    ChunkPos chunkPos     = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                    auto     neighborIter = this->netherBsciChunkData.find(chunkPos);
                    if (neighborIter == this->netherBsciChunkData.end()) continue;
                    neighborIter->second.neighborValidCount--;
                    if (!neighborIter->second.neighborValidCount && !neighborIter->second.dataDrawed) {
                        geometryGroup->remove(neighborIter->second.chunkSavedDrawGeoId);
                        toRemove.emplace_back(chunkPos);
                    }
                }
            }
            if (data.ancientDebrisGeoId.value) geometryGroup->remove(data.ancientDebrisGeoId);
            if (!data.neighborValidCount) {
                geometryGroup->remove(data.chunkSavedDrawGeoId);
                toRemove.emplace_back(originChunkPos);
            }
            data.dataDrawed = false;
            continue;
        }
        if (data.chunkSaved || (!data.neighborValidCount && !data.dataDrawed)) continue;
        // auto chunk = netherDim->getBlockSourceFromMainChunkSource().getChunk(originChunkPos);
        // if (dbChunkStorage->isChunkSaved(originChunkPos)
        // || (chunk && *chunk->mLoadState == ChunkState::Loaded && !chunk->isNonActorDataDirty())) {
        if (dbChunkStorage->isChunkSaved(originChunkPos)) {
            data.chunkSaved = true;
            geometryGroup->remove(data.chunkSavedDrawGeoId);

            if (data.chunkSavedDrawGeoId.value) {
                mod().getLogger().warn(
                    "Chunk {} load state changed but still has debug geometry, something may be wrong, pos5",
                    originChunkPos.toString()
                );
            }
            data.chunkSavedDrawGeoId = geometryGroup->box(
                netherDim->getDimensionId(),
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
            );
        }
    }
    for (const auto& key : toRemove) {
        this->netherBsciChunkData.erase(key);
    }
}

void DuplicatableManager::setShowType(ShowType _showType, bool show) {
    if (((this->showType & static_cast<uint>(_showType)) != 0) == show) return;
    if (show) {
        this->showType    |= static_cast<uint>(_showType);
        this->tickCounter  = 0;
    } else {
        this->showType &= ~static_cast<uint>(_showType);
        this->removeBsciData(_showType);
    }
}

bool DuplicatableManager::getShowType(ShowType _showType) { return this->showType & static_cast<uint>(_showType); }

void DuplicatableManager::hook(bool enable) {
    if (enable) {
        DuplicatableHook1::hook();
        DuplicatableHook2::hook();
    } else {
        DuplicatableHook1::unhook();
        DuplicatableHook2::unhook();
    }
}

std::string DuplicatableManager::test(ChunkPos chunkPos) {
    std::string     res = chunkPos.toString() + "\n";
    std::lock_guard lock(this->netherDataMapLock);
    auto            iter = this->netherDataMap.find(chunkPos);
    if (iter != this->netherDataMap.end()) {
        res += "netherDataMap: len(ancientDebrisPosSet) = " + std::to_string(iter->second.ancientDebrisPosSet.size())
             + '\n';
    } else {
        res += "netherDataMap数据不存在";
    }
    auto it = this->netherBsciChunkData.find(chunkPos);
    if (it != this->netherBsciChunkData.end()) {
        res += "netherBsciChunkData: dataDrawed = " + std::to_string(it->second.dataDrawed)
             + " neighborValidCount = " + std::to_string(it->second.neighborValidCount);
    } else {
        res += "netherBsciChunkData数据不存在";
    }
    return res;
}
} // namespace coral_fans::functions::locate