#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/service/Bedrock.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/resources/persona/color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/chunk/LevelChunkVolumeData.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/v1/HardcodedSpawnAreaType.h"
#include "mc/world/level/levelgen/v1/StructureSpawnRegistry.h"
#include "mc/world/level/levelgen/v2/StructureSpawnOverride.h"
#include "mc/world/phys/AABB.h"
#include <memory>
#include <string>
#include <vector>


namespace {

static const int radius = 5;

} // namespace

namespace coral_fans::functions {

void HsaManager::drawHsa() {
    static std::array colors{
        mce::Color::BLACK(),
        mce::Color::BLUE(),
        mce::Color::CYAN(),
        mce::Color::GREEN(),
        mce::Color::GREY(),
        mce::Color::MINECOIN_GOLD(),
        mce::Color::ORANGE(),
        mce::Color::PINK(),
        mce::Color::PURPLE(),
        mce::Color::REBECCA_PURPLE(),
        mce::Color::RED(),
        mce::Color::YELLOW()
    };
    auto level = ll::service::getLevel();
    if (level.has_value()) {
        level->forEachPlayer([&](Player& player) {
            auto  originChunkPos = utils::blockPosToChunkPos(((const Actor&)player).getFeetBlockPos());
            int   dim            = ((const Actor&)player).getDimensionId();
            auto& region         = ((const Actor&)player).getDimension().getChunkSource();
            // chunks
            for (int _i = -radius; _i <= radius; ++_i) {
                for (int j = -radius; j <= radius; ++j) {
                    ChunkPos chunkPos = ChunkPos(originChunkPos.x + _i, originChunkPos.z + j);
                    auto     _it      = mChunkLoaded[dim].find(chunkPos);
                    if (!(_it == mChunkLoaded[dim].end())) {
                        _it->second = true;
                        continue;
                    }
                    mChunkLoaded[dim][chunkPos] = true;
                    auto chunk                  = region.getExistingChunk(chunkPos);
                    if (chunk && chunk->isFullyLoaded()) {
                        ::std::vector<::BlockPos>               hsa = chunk->mLevelChunkVolumeData->structureSpawnPos();
                        std::unordered_map<BlockPos, short>     count;
                        std::vector<bsci::GeometryGroup::GeoId> geometryGroup;
                        for (auto i : hsa) {
                            auto  it     = count.find(i);
                            short _count = 0;
                            if (it == count.end()) count[i] = 1;
                            else {
                                _count     = it->second;
                                it->second = (it->second + 1) % 12;
                            }
                            auto& mod = coral_fans::mod();

                            auto color = colors[_count];
                            auto ids   = std::array{
                                mod.getGeometryGroup()
                                    ->line(dim, {i.x, i.y, i.z}, {i.x, i.y + 1, i.z}, mce::Color::WHITE()),
                                mod.getGeometryGroup()->line(dim, {i.x + 1, i.y, i.z}, {i.x + 1, i.y + 1, i.z}, color),
                                mod.getGeometryGroup()->line(dim, {i.x, i.y, i.z + 1}, {i.x, i.y + 1, i.z + 1}, color),
                                mod.getGeometryGroup()
                                    ->line(dim, {i.x + 1, i.y, i.z + 1}, {i.x + 1, i.y + 1, i.z + 1}, color),
                                mod.getGeometryGroup()->line(dim, {i.x, i.y + 1, i.z}, {i.x + 1, i.y + 1, i.z}, color),
                                mod.getGeometryGroup()->line(dim, {i.x, i.y + 1, i.z}, {i.x, i.y + 1, i.z + 1}, color),
                                mod.getGeometryGroup()
                                    ->line(dim, {i.x + 1, i.y + 1, i.z}, {i.x + 1, i.y + 1, i.z + 1}, color),
                                mod.getGeometryGroup()
                                    ->line(dim, {i.x, i.y + 1, i.z + 1}, {i.x + 1, i.y + 1, i.z + 1}, color),
                            };
                            geometryGroup.push_back(mod.getGeometryGroup()->merge(ids));
                        }
                        mParticleMap[dim][chunkPos] = geometryGroup;
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
    for (int dim = 0; dim < 3; dim++) {
        for (auto i : mParticleMap[dim]) {
            for (auto j : i.second) coral_fans::mod().getGeometryGroup()->remove(j);
        }
        mParticleMap[dim].clear();
        mChunkLoaded[dim].clear();
    }
}

void HsaManager::runtimeRemove() {
    for (int dim = 0; dim < 3; dim++) {
        auto it = mChunkLoaded[dim].begin();
        while (it != mChunkLoaded[dim].end()) {
            if (!it->second) {
                auto geometryGroup = mParticleMap[dim].find(it->first);
                for (auto j : geometryGroup->second) coral_fans::mod().getGeometryGroup()->remove(j);
                mParticleMap[dim].erase(geometryGroup);
                it = mChunkLoaded[dim].erase(it);
            } else {
                it->second = false;
                it++;
            }
        }
    }
}

} // namespace coral_fans::functions