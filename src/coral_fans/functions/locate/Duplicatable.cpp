#include "DuplicatableManager.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/WorldBlockTarget.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/SubChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/feature/NoSurfaceOreFeature.h"
#include "mc/world/level/levelgen/feature/OreFeature.h"
#include "mc/world/level/levelgen/v1/NetherGenerator.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <memory>
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
    auto            dim                 = neighborhood.mDimension;
    DBChunkStorage* dbChunkStorage      = static_cast<DBChunkStorage*>(&(*dim->mChunkSource->mOwnedParent));
    auto&           pos                 = neighborhood.mArea->mBounds.mMin;
    ChunkPos        originChunkPos      = ChunkPos(pos->x + 1, pos->z + 1);
    auto&           duplicatableManager = DuplicatableManager::getInstance();
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
            if (!dbChunkStorage->isChunkSaved(chunkPos)) {
                auto threadData   = std::make_unique<NetherThreadTemperaryData>();
                threadData->chunk = neighborhood.getExistingChunk(originChunkPos).get();
                auto threadId     = std::this_thread::get_id();
                {
                    std::lock_guard lock(duplicatableManager.netherDecorationThreadIdsLock);
                    duplicatableManager.netherDecorationThreadIds.emplace(threadId, std::move(threadData));
                }
                auto ori = origin(neighborhood);
                {
                    std::lock_guard lock(duplicatableManager.netherDecorationThreadIdsLock);
                    auto            it = duplicatableManager.netherDecorationThreadIds.find(threadId);
                    std::lock_guard lock2(duplicatableManager.netherDataMapLock);
                    if (it->second->isEmpty) duplicatableManager.netherDataMap.erase(originChunkPos);
                    else {
                        auto [newIter, inserted] = duplicatableManager.netherDataMap.insert_or_assign(
                            originChunkPos,
                            std::move(it->second->threadData)
                        );
                        if (!inserted) newIter->second.reload = true;
                    }
                    duplicatableManager.netherDecorationThreadIds.erase(it);
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
    auto                       threadId            = std::this_thread::get_id();
    auto&                      duplicatableManager = DuplicatableManager::getInstance();
    NetherThreadTemperaryData* threadData          = nullptr;
    {
        std::lock_guard lock(duplicatableManager.netherDecorationThreadIdsLock);
        auto            it = duplicatableManager.netherDecorationThreadIds.find(threadId);
        if (it != duplicatableManager.netherDecorationThreadIds.end()) threadData = it->second.get();
    }
    if (!threadData) return origin(context);
    threadData->worldBlockTargetShouldOperate = true;
    auto ori                                  = origin(context);
    threadData->worldBlockTargetShouldOperate = false;
    if (!threadData->temeraryPoses.empty()) {
        threadData->threadData.netheritePosMap.emplace(context.mPos, std::move(threadData->temeraryPoses));
        threadData->isEmpty = false;
    }
    return ori;
}

LL_TYPE_INSTANCE_HOOK(
    DuplicatableManager::DuplicatableHook3,
    ll::memory::HookPriority::Normal,
    WorldBlockTarget,
    &WorldBlockTarget ::$setBlock,
    bool,
    ::BlockPos const& pos,
    ::Block const&    block,
    int               flag
) {
    auto  threadId            = std::this_thread::get_id();
    auto& duplicatableManager = DuplicatableManager::getInstance();
    auto  ori                 = origin(pos, block, flag);
    if (ori) [[likely]] {
        NetherThreadTemperaryData* threadData = nullptr;
        {
            std::lock_guard lock(duplicatableManager.netherDecorationThreadIdsLock);
            auto            it = duplicatableManager.netherDecorationThreadIds.find(threadId);
            if (it == duplicatableManager.netherDecorationThreadIds.end() || !it->second->worldBlockTargetShouldOperate)
                return ori;
            else threadData = it->second.get();
        }
        if (ChunkPos(pos) != threadData->chunk->mPosition) threadData->temeraryPoses.emplace_back(pos);
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

bsci::GeometryGroup::GeoId DuplicatableManager::drawNetherite(std::map<BlockPos, std::vector<BlockPos>>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup   = coral_fans::mod().getGeometryGroup();
    auto& netheriteConfig = coral_fans::mod().getConfig().locate.duplicatable.netherite;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(5 * data.size());
    for (auto& [oriPos, poses] : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(oriPos, oriPos + BlockPos(1, 1, 1)), mce::Color(netheriteConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
            "translate.locate.duplicatable.netherite.oriPos"_tr(),
            mce::Color(netheriteConfig.textColor)
        ));
        for (auto& pos : poses) {
            geoIdList.emplace_back(
                geometryGroup->box(1, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(netheriteConfig.posColor))
            );
            geoIdList.emplace_back(geometryGroup->text(
                1,
                {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
                "translate.locate.duplicatable.netherite.pos"_tr(),
                mce::Color(netheriteConfig.textColor)
            ));
            geoIdList.emplace_back(geometryGroup->arrow(
                1,
                {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
                {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
                mce::Color(netheriteConfig.arrowColor),
                0.75f,
                0.2f
            ));
        }
    }
    return geometryGroup->merge(geoIdList);
}

bool DuplicatableManager::isChunkValid(BlockSource& region, ChunkPos originChunkPos) {
    DBChunkStorage* dbChunkStorage = static_cast<DBChunkStorage*>(&(*region.getDimension().mChunkSource->mOwnedParent));
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
            if (!dbChunkStorage->isChunkSaved(chunkPos)) return true;
        }
    }
    return false;
}

void DuplicatableManager::tryRemoveNetherChunkData(ChunkPos originChunkPos) {
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    auto  originIter    = this->netherBsciChunkData.find(originChunkPos);
    if (originIter != this->netherBsciChunkData.end() && originIter->second.dataDrawed) {
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
        if (originIter->second.netheriteGeoId.value) geometryGroup->remove(originIter->second.netheriteGeoId);
        if (!originIter->second.neighborValidCount) {
            geometryGroup->remove(originIter->second.chunkSavedDrawGeoId);
            this->netherBsciChunkData.erase(originIter);
        } else originIter->second.dataDrawed = 0;
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
                if (!dbChunkStorage->isChunkSaved(chunkPos)) {
                    neighborIter->second.chunkSaved          = false;
                    neighborIter->second.chunkSavedDrawGeoId = geometryGroup->box(
                        region.getDimensionId(),
                        {Vec3(chunkPos.x * 16 + 0.1, 0, chunkPos.z * 16 + 0.1),
                         Vec3(chunkPos.x * 16 + 15.9, 128, chunkPos.z * 16 + 15.9)},
                        mce::Color(duplicatableConfig.chunkSavedDebugInfo.unsavedChunk)
                    );
                } else {
                    neighborIter->second.chunkSaved          = true;
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
        if (!dbChunkStorage->isChunkSaved(originChunkPos)) {
            originChunkData.chunkSaved          = false;
            originChunkData.chunkSavedDrawGeoId = geometryGroup->box(
                region.getDimensionId(),
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.unsavedChunk)
            );
        } else {
            originChunkData.chunkSaved          = true;
            originChunkData.chunkSavedDrawGeoId = geometryGroup->box(
                region.getDimensionId(),
                {Vec3(originChunkPos.x * 16 + 0.1, 0, originChunkPos.z * 16 + 0.1),
                 Vec3(originChunkPos.x * 16 + 15.9, 128, originChunkPos.z * 16 + 15.9)},
                mce::Color(duplicatableConfig.chunkSavedDebugInfo.savedChunk)
            );
        }
    }
}

void DuplicatableManager::netherDraw(BlockSource& region, ChunkPos chunkPos, NetherData& data) {
    auto [it, inserted] = this->netherBsciChunkData.try_emplace(chunkPos);
    if (!inserted && data.reload && it->second.dataDrawed) {
        auto& geometryGroup = coral_fans::mod().getGeometryGroup();
        if (it->second.netheriteGeoId.value) {
            geometryGroup->remove(it->second.netheriteGeoId);
            it->second.netheriteGeoId.value = 0;
        }
        it->second.dataDrawed = 0;
    } else this->drawChunkSavedInfo(region, chunkPos, it->second);
    it->second.runtimeRemoveTickCounter = 0;
    data.reload                         = false;

    if (this->showType & static_cast<uint>(ShowType::Netherite) && data.netheritePosMap.size()
        && !it->second.netheriteGeoId.value) {
        it->second.netheriteGeoId  = this->drawNetherite(data.netheritePosMap);
        it->second.dataDrawed     |= static_cast<uint>(ShowType::Netherite);
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
                    auto            it = this->netherDataMap.find(chunkPos);
                    if (it == this->netherDataMap.end()) {
                        this->tryRemoveNetherChunkData(chunkPos);
                        continue;
                    }
                    if (!(this->showType & static_cast<uint>(ShowType::Netherite) && it->second.netheritePosMap.size()
                          // ||
                        ))
                        continue;
                    if (!this->isChunkValid(region, chunkPos)) {
                        this->netherDataMap.erase(it);
                        this->tryRemoveNetherChunkData(chunkPos);
                        continue;
                    }
                    netherDraw(region, chunkPos, it->second);
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
            if (chunkData.netheriteGeoId.value) geometryGroup->remove(chunkData.netheriteGeoId);
            if (chunkData.chunkSavedDrawGeoId.value) geometryGroup->remove(chunkData.chunkSavedDrawGeoId);
        }
        this->netherBsciChunkData.clear();
    }
    switch (_showType) {
    case ShowType::Netherite:
        std::erase_if(this->netherBsciChunkData, [&geometryGroup](auto& data) {
            if (data.second.netheriteGeoId.value) {
                geometryGroup->remove(data.second.netheriteGeoId);
                data.second.netheriteGeoId.value = 0;
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
            if (data.netheriteGeoId.value) {
                geometryGroup->remove(data.netheriteGeoId);
                data.netheriteGeoId.value = 0;
            }
            if (!data.neighborValidCount) {
                geometryGroup->remove(data.chunkSavedDrawGeoId);
                toRemove.emplace_back(originChunkPos);
            }
            data.dataDrawed = 0;
            continue;
        }
        if (data.chunkSaved || (!data.neighborValidCount && !data.dataDrawed)) continue;
        if (dbChunkStorage->isChunkSaved(originChunkPos)) {
            data.chunkSaved = true;
            geometryGroup->remove(data.chunkSavedDrawGeoId);
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
        DuplicatableHook3::hook();
    } else {
        DuplicatableHook1::unhook();
        DuplicatableHook2::unhook();
        DuplicatableHook3::unhook();
    }
}

std::string DuplicatableManager::test(ChunkPos chunkPos) {
    std::string     res = chunkPos.toString() + "\n";
    std::lock_guard lock(this->netherDataMapLock);
    auto            iter = this->netherDataMap.find(chunkPos);
    if (iter != this->netherDataMap.end()) {
        res += "netherDataMap: len(netheritePosSet) = " + std::to_string(iter->second.netheritePosMap.size()) + '\n';
    } else {
        res += "netherDataMap数据不存在\n";
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