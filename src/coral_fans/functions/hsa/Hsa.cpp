#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/service/Bedrock.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/deps/core/math/Color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/ChunkState.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/chunk/LevelChunkVolumeData.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/v2/SpawnerData.h"
#include "mc/world/level/levelgen/v2/StructureSpawnOverride.h"
#include <memory>
#include <vector>

namespace coral_fans::functions {
void HsaManager::setHsaShow(bool show) {
    if (this->hsaShow == show) return;
    this->hsaShow = show;
    if (show) {
        this->tickCounter              = 0;
        this->runtimeRemoveTickCounter = 1;
    } else this->remove();
}

void HsaManager::setStructureShow(bool show) {
    if (this->structureShow == show) return;
    this->structureShow = show;
    if (show) {
        this->tickCounter              = 0;
        this->runtimeRemoveTickCounter = 1;
    } else this->remove();
}

bool HsaManager::getHsaShow() { return this->hsaShow; }

bool HsaManager::getStructureShow() { return this->structureShow; }

void HsaManager::drawChunkHsa(std::vector<::BlockPos>& hsa, DimensionType dim, HsaChunkData& chunkData) {
    std::unordered_map<BlockPos, int> hsaCount;
    auto&                             hsaConfig = mod().getConfig().functions.hsa;
    for (auto& pos : hsa) {
        auto [it, inserted2] = hsaCount.try_emplace(pos);
        if (inserted2) it->second = 1;
        else it->second++;
    }
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(8 * hsaCount.size() + 1);
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    for (auto& [pos, count] : hsaCount) {
        geoIdList.emplace_back(
            geometryGroup
                ->line(dim, {pos.x, pos.y, pos.z}, {pos.x, pos.y + 1, pos.z}, mce::Color(hsaConfig.hsaNorthWestColor))
        );
        geoIdList.emplace_back(
            geometryGroup
                ->line(dim, {pos.x + 1, pos.y, pos.z}, {pos.x + 1, pos.y + 1, pos.z}, mce::Color(hsaConfig.hsaColor))
        );
        geoIdList.emplace_back(
            geometryGroup
                ->line(dim, {pos.x, pos.y, pos.z + 1}, {pos.x, pos.y + 1, pos.z + 1}, mce::Color(hsaConfig.hsaColor))
        );
        geoIdList.emplace_back(geometryGroup->line(
            dim,
            {pos.x + 1, pos.y, pos.z + 1},
            {pos.x + 1, pos.y + 1, pos.z + 1},
            mce::Color(hsaConfig.hsaColor)
        ));
        geoIdList.emplace_back(
            geometryGroup
                ->line(dim, {pos.x, pos.y + 1, pos.z}, {pos.x + 1, pos.y + 1, pos.z}, mce::Color(hsaConfig.hsaColor))
        );
        geoIdList.emplace_back(
            geometryGroup
                ->line(dim, {pos.x, pos.y + 1, pos.z}, {pos.x, pos.y + 1, pos.z + 1}, mce::Color(hsaConfig.hsaColor))
        );
        geoIdList.emplace_back(geometryGroup->line(
            dim,
            {pos.x + 1, pos.y + 1, pos.z},
            {pos.x + 1, pos.y + 1, pos.z + 1},
            mce::Color(hsaConfig.hsaColor)
        ));
        geoIdList.emplace_back(geometryGroup->line(
            dim,
            {pos.x, pos.y + 1, pos.z + 1},
            {pos.x + 1, pos.y + 1, pos.z + 1},
            mce::Color(hsaConfig.hsaColor)
        ));
        if (count > 1) {
            geoIdList.emplace_back(geometryGroup->text(
                dim,
                {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
                "x" + std::to_string(count),
                mce::Color::WHITE(),
                1.0f
            ));
        }
    }
    chunkData.hsaGeoId = geometryGroup->merge(geoIdList);
}

void HsaManager::drawChunkStructure(
    entt::basic_storage<::br::ChunkBoundingBox, ::br::ChunkEntity, ::std::allocator<::br::ChunkBoundingBox>, void>&
                  chunkBoundingBoxes,
    DimensionType dim,
    HsaChunkData& chunkData,
    ChunkPos      chunkPos
) {
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    auto&                                   hsaConfig = mod().getConfig().functions.hsa;
    geoIdList.reserve(chunkBoundingBoxes.size());
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    for (auto [entity, component] : chunkBoundingBoxes.each()) {
        if (ChunkPos(component.box->min) != chunkPos || ChunkPos(component.box->max) != chunkPos
            || component.box->min.y < -64 || component.box->max.y > 320) {
            continue;
        }
        geoIdList.emplace_back(geometryGroup->box(
            dim,
            AABB(component.box->min, component.box->max + BlockPos(1, 1, 1)),
            mce::Color(hsaConfig.structureColor)
        ));
    }
    chunkData.structureGeoId = geometryGroup->merge(geoIdList);
}

void HsaManager::draw() {
    if (!this->hsaShow && !this->structureShow) return;
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;
    static int radius = std::max(0, mod().getConfig().functions.hsa.drawRadius);
    level->forEachPlayer([this, radius = radius](Player& player) {
        ChunkPos originChunkPos = ChunkPos(player.getFeetBlockPos());
        auto&    dim            = player.getDimension();
        int      dimId          = player.getDimensionId();
        for (int i = -radius; i <= radius; ++i) {
            int maxJ = radius - abs(i);
            for (int j = -maxJ; j <= maxJ; ++j) {
                ChunkPos chunkPos = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                auto     chunk    = (*dim.mBlockSource)->getChunk(chunkPos);
                if (chunk && *chunk->mLoadState == ChunkState::Loaded) {
                    std::vector<::BlockPos> hsa;
                    auto& chunkBoundingBoxes = std::get<1>(*chunk->mLevelChunkVolumeData->mDataRegistry->mData);
                    if (this->hsaShow) hsa = chunk->mLevelChunkVolumeData->structureSpawnPos();

                    if (hsa.size() || (this->structureShow && chunkBoundingBoxes.size())) {

                        auto [iter, inserted] = this->mChunkDataMap.try_emplace(std::make_pair(chunkPos, dimId));
                        if (hsa.size() && !iter->second.hsaGeoId.value) this->drawChunkHsa(hsa, dimId, iter->second);
                        try {
                            if ((hsa.size() || this->structureShow) && chunkBoundingBoxes.size()
                                && !iter->second.structureGeoId.value)
                                this->drawChunkStructure(chunkBoundingBoxes, dimId, iter->second, chunkPos);
                            iter->second.freshed = true;
                        } catch (const std::runtime_error&) {
                            if (iter->second.hsaGeoId.value) {
                                coral_fans::mod().getGeometryGroup()->remove(iter->second.hsaGeoId);
                            }
                            this->mChunkDataMap.erase(iter);
                        }
                    }
                }
            }
        }
        return true;
    });
}

void HsaManager::tick() {
    if (!this->tickCounter) {
        this->draw();
        if (!this->runtimeRemoveTickCounter) {
            this->runtimeRemove();
        }
        static int removeInterval      = std::max(1, mod().getConfig().functions.hsa.runtimeRemoveScale);
        this->runtimeRemoveTickCounter = (this->runtimeRemoveTickCounter + 1) % removeInterval;
    }
    static int interval = std::max(1, mod().getConfig().functions.hsa.drawInterval);
    this->tickCounter   = (this->tickCounter + 1) % interval;
}

void HsaManager::remove() {
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    if (!this->hsaShow) {
        if (!this->structureShow) {
            for (auto& [_, chunkData] : this->mChunkDataMap) {
                if (chunkData.hsaGeoId.value) geometryGroup->remove(chunkData.hsaGeoId);
                if (chunkData.structureGeoId.value) geometryGroup->remove(chunkData.structureGeoId);
            }
            this->mChunkDataMap.clear();
        } else {
            std::erase_if(this->mChunkDataMap, [&geometryGroup](auto& data) {
                if (data.second.hsaGeoId.value) {
                    geometryGroup->remove(data.second.hsaGeoId);
                    data.second.hsaGeoId.value = 0;
                }
                return !data.second.structureGeoId.value;
            });
        }
    } else {
        std::erase_if(this->mChunkDataMap, [&geometryGroup](auto& data) {
            if (!data.second.hsaGeoId.value) {
                geometryGroup->remove(data.second.structureGeoId);
                return true;
            }
            return false;
        });
    }
}

void HsaManager::runtimeRemove() {
    auto& geometryGroup = coral_fans::mod().getGeometryGroup();
    std::erase_if(this->mChunkDataMap, [&geometryGroup](auto& data) {
        if (data.second.freshed) {
            data.second.freshed = false;
            return false;
        }
        if (data.second.hsaGeoId.value) geometryGroup->remove(data.second.hsaGeoId);
        if (data.second.structureGeoId.value) geometryGroup->remove(data.second.structureGeoId);
        return true;
    });
}

std::vector<BlockPos> HsaManager::listChunkHsa(BlockSource& region, ChunkPos chunkPos) {
    auto chunk = region.getChunk(chunkPos);
    if (chunk && *chunk->mLoadState == ChunkState::Loaded) {
        return chunk->mLevelChunkVolumeData->structureSpawnPos();
    }
    return {};
}

std::vector<AABB> HsaManager::listChunkStructure(BlockSource& region, ChunkPos chunkPos) {
    auto chunk = region.getChunk(chunkPos);
    if (chunk && *chunk->mLoadState == ChunkState::Loaded) {
        std::vector<AABB> result;
        auto&             chunkBoundingBoxes = std::get<1>(*chunk->mLevelChunkVolumeData->mDataRegistry->mData);
        result.reserve(chunkBoundingBoxes.size());
        for (auto [entity, component] : chunkBoundingBoxes.each()) {
            if (ChunkPos(component.box->min) != chunkPos || ChunkPos(component.box->max) != chunkPos
                || component.box->min.y < -64 || component.box->max.y > 320) {
                continue;
            }
            result.emplace_back(AABB(component.box->min, component.box->max + BlockPos(1, 1, 1)));
        }
        return result;
    }
    return {};
}
} // namespace coral_fans::functions