#include "DuplicatableManager.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/feature/NoSurfaceOreFeature.h"
#include "mc/world/level/levelgen/v1/NetherGenerator.h"
#include "mc/world/level/storage/DBChunkStorage.h"
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
    auto&           region         = dim->getBlockSourceFromMainChunkSource();
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) {
                ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
                auto     chunk    = region.getChunk(chunkPos);
                if (chunk->isNonActorDataDirty() && !dbChunkStorage->isChunkSaved(chunkPos)) {
                    auto& DuplicatableManager = DuplicatableManager::getInstance();
                    auto  threadId            = std::this_thread::get_id();
                    {
                        std::lock_guard lock(DuplicatableManager.netherDecorationThreadIdsLock);
                        DuplicatableManager.netherDecorationThreadIds.push_back(threadId);
                    }
                    auto ori = origin(neighborhood);
                    {
                        std::lock_guard lock(DuplicatableManager.netherDecorationThreadIdsLock);
                        std::erase_if(
                            DuplicatableManager.netherDecorationThreadIds,
                            [threadId](const std::thread::id& id) { return id == threadId; }
                        );
                    }
                    return ori;
                }
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
    if (!ori.has_value()) {
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
        auto& netherMainBlockSource = netherDim->getBlockSourceFromMainChunkSource();
        {
            std::lock_guard lock(this->netherDataMapLock);
            std::erase_if(this->netherDataMap, [&netherMainBlockSource](auto&& iter) {
                return !netherMainBlockSource.getChunk(iter.first);
            });
        }
    }
}

void DuplicatableManager::draw() {
    if (!static_cast<int>(this->showType)) return;
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    auto& locateConfig  = coral_fans::mod().getConfig().locate;
    level->forEachPlayer([this, &geometryGroup, &locateConfig](Player& player) {
        int dimId = player.getDimensionId();
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
                    auto [netherBsciChunkDataIter, inserted] = this->netherBsciChunkData.try_emplace(chunkPos);
                    if (this->showType & static_cast<uint>(ShowType::AncientDebris)
                        && netherDataMapIter->second.ancientDebrisPosSet.size()
                        && !netherBsciChunkDataIter->second.ancientDebrisGeoId.value) {
                        std::vector<bsci::GeometryGroup::GeoId> geoIdList;
                        geoIdList.reserve(4 * netherDataMapIter->second.ancientDebrisPosSet.size());
                        for (auto& [oriPos, endPos] : netherDataMapIter->second.ancientDebrisPosSet) {
                            geoIdList.emplace_back(geometryGroup->box(
                                1,
                                AABB(oriPos, oriPos + BlockPos(1, 1, 1)),
                                mce::Color(locateConfig.duplicatable.ancientDebris.originPosColor)
                            ));
                            geoIdList.emplace_back(geometryGroup->text(
                                1,
                                {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
                                "translate.locate.duplicatable.ancientDebris.oriPos"_tr()
                            ));
                            geoIdList.emplace_back(geometryGroup->box(
                                1,
                                AABB(endPos, endPos + BlockPos(1, 1, 1)),
                                mce::Color(locateConfig.duplicatable.ancientDebris.endPosColor)
                            ));
                            geoIdList.emplace_back(geometryGroup->text(
                                1,
                                {endPos.x + 0.5f, endPos.y + 0.5f, endPos.z + 0.5f},
                                "translate.locate.duplicatable.ancientDebris.endPos"_tr()
                            ));
                        }
                        netherBsciChunkDataIter->second.ancientDebrisGeoId = geometryGroup->merge(geoIdList);
                    }
                }
            }
        }
        return true;
    });
}

void DuplicatableManager::tick() {
    auto& locateConfig = mod().getConfig().locate;
    if (!this->tickCounter) {
        this->draw();
        if (!this->runtimeRemoveTickCounter) {
            this->bsciDataRuntimeRemove();
        }
        this->runtimeRemoveTickCounter =
            (this->runtimeRemoveTickCounter + 1) % locateConfig.duplicatable.runtimeRemoveScale;
    }
    this->tickCounter = (this->tickCounter + 1) % locateConfig.duplicatable.drawInterval;
}

void DuplicatableManager::removeBsciData(ShowType showType) {
    auto& geometryGroup  = coral_fans::mod().getGeometryGroup();
    this->showType      &= ~static_cast<uint>(showType);
    if (!this->showType) {
        for (auto& [_, chunkData] : this->netherBsciChunkData) {
            if (chunkData.ancientDebrisGeoId.value) geometryGroup->remove(chunkData.ancientDebrisGeoId);
        }
        this->netherBsciChunkData.clear();
    }
    switch (showType) {
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
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    std::erase_if(this->netherBsciChunkData, [&geometryGroup](auto& data) {
        if (data.second.freshed) {
            data.second.freshed = false;
            return false;
        }
        if (data.second.ancientDebrisGeoId.value) geometryGroup->remove(data.second.ancientDebrisGeoId);
        return true;
    });
}


void DuplicatableManager::setShowType(ShowType showType, bool show) {
    if (this->showType & static_cast<uint>(showType)) return;
    this->showType |= static_cast<uint>(showType);
    if (show) {
        this->tickCounter              = 0;
        this->runtimeRemoveTickCounter = 1;
    } else this->removeBsciData(showType);
}

bool DuplicatableManager::getShowType(ShowType showType) { return this->showType & static_cast<uint>(showType); }

void DuplicatableManager::hook(bool enable) {
    if (enable) {
        DuplicatableHook1::hook();
        DuplicatableHook2::hook();
    } else {
        DuplicatableHook1::unhook();
        DuplicatableHook2::unhook();
    }
}
} // namespace coral_fans::functions::locate