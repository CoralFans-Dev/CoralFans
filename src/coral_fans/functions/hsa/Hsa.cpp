#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/service/Bedrock.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/deps/core/math/Color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/chunk/LevelChunkVolumeData.h"
#include "mc/world/level/dimension/Dimension.h"
#include <memory>
#include <vector>

namespace {

static const int radius = 5;

} // namespace

namespace coral_fans::functions {

void HsaManager::drawHsa() {
    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& player) {
            ChunkPos originChunkPos = utils::blockPosToChunkPos(((const Actor&)player).getFeetBlockPos());
            int      dim            = ((const Actor&)player).getDimensionId();
            auto&    region         = *((const Actor&)player).getDimension().mBlockSource;
            // chunks
            for (int _i = -radius; _i <= radius; ++_i) {
                for (int j = -radius; j <= radius; ++j) {
                    ChunkPos chunkPos = ChunkPos(originChunkPos.x + _i, originChunkPos.z + j);
                    auto     chunk    = region->getChunk(chunkPos);
                    if (chunk && chunk->mLoadState.get() == ChunkState::Loaded) {
                        ::std::vector<::BlockPos> hsa = chunk->mLevelChunkVolumeData->structureSpawnPos();
                        if (!hsa.size()) continue; // hsa数量为0则跳过
                        auto [iter, inserted] = this->mParticleMap.try_emplace(std::make_pair(chunkPos, dim));
                        iter->second.second   = true;
                        if (inserted) {
                            std::unordered_map<BlockPos, int> hsaCount;
                            for (auto& pos : hsa) {
                                auto [it, inserted2] = hsaCount.try_emplace(pos);
                                if (inserted2) it->second = 1;
                                else it->second++;
                                coral_fans::mod().getLogger().info(
                                    "Found HSA at position {}, count: {}",
                                    pos,
                                    it->second
                                );
                            }
                            std::vector<bsci::GeometryGroup::GeoId> geoIdList;
                            for (auto& [pos, count] : hsaCount) {
                                auto& geometryGroup = coral_fans::mod().getGeometryGroup();
                                auto  ids           = std::vector{
                                    geometryGroup->line(
                                        dim,
                                        {pos.x, pos.y, pos.z},
                                        {pos.x, pos.y + 1, pos.z},
                                        mce::Color::WHITE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x + 1, pos.y, pos.z},
                                        {pos.x + 1, pos.y + 1, pos.z},
                                        mce::Color::BLUE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x, pos.y, pos.z + 1},
                                        {pos.x, pos.y + 1, pos.z + 1},
                                        mce::Color::BLUE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x + 1, pos.y, pos.z + 1},
                                        {pos.x + 1, pos.y + 1, pos.z + 1},
                                        mce::Color::BLUE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x, pos.y + 1, pos.z},
                                        {pos.x + 1, pos.y + 1, pos.z},
                                        mce::Color::BLUE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x, pos.y + 1, pos.z},
                                        {pos.x, pos.y + 1, pos.z + 1},
                                        mce::Color::BLUE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x + 1, pos.y + 1, pos.z},
                                        {pos.x + 1, pos.y + 1, pos.z + 1},
                                        mce::Color::BLUE()
                                    ),
                                    geometryGroup->line(
                                        dim,
                                        {pos.x, pos.y + 1, pos.z + 1},
                                        {pos.x + 1, pos.y + 1, pos.z + 1},
                                        mce::Color::BLUE()
                                    ),
                                };
                                if (count > 1) {
                                    ids.push_back(geometryGroup->text(
                                        dim,
                                        {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
                                        "x" + std::to_string(count),
                                        mce::Color::WHITE(),
                                        1.0f
                                    ));
                                }
                                geoIdList.push_back(geometryGroup->merge(ids));
                            }
                            iter->second.first = std::move(geoIdList);
                        }
                    }
                }
            }
            return true;
        });
    }
}

void HsaManager::tick() {
    static int gt = 0, gt2 = 1;
    if (!mShow) return;
    if (!gt) {
        drawHsa();
        if (!gt2) runtimeRemove();
        gt2 = (gt2 + 1) % 15;
    }
    gt = (gt + 1) % 80;
}

void HsaManager::remove() {
    for (auto i : mParticleMap) {
        for (auto j : i.second.first) coral_fans::mod().getGeometryGroup()->remove(j);
    }
    mParticleMap.clear();
}

void HsaManager::runtimeRemove() {
    std::erase_if(mParticleMap, [](auto& data) {
        if (!data.second.second) {
            for (auto j : data.second.first) coral_fans::mod().getGeometryGroup()->remove(j);
            return true;
        } else {
            data.second.second = false;
            return false;
        }
    });
}

} // namespace coral_fans::functions