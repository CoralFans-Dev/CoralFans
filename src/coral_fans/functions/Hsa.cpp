#include "coral_fans/functions/Hsa.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/service/Bedrock.h"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/resources/persona/color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/v1/HardcodedSpawnAreaType.h"
#include "mc/world/phys/AABB.h"
#include <memory>
#include <string>
#include <vector>


namespace {

static const int radius = 5;

auto getSpawnAreaFromHSA = [](AABB& aabb) {
    return AABB{
        BlockPos{
                 (aabb.max.x - aabb.min.x + 1) / 2 + aabb.min.x,
                 aabb.min.y,
                 (aabb.max.z - aabb.min.z + 1) / 2 + aabb.min.z
        },
        BlockPos{
                 (aabb.max.x - aabb.min.x + 1) / 2 + aabb.min.x + 1,
                 aabb.max.y + 1,
                 (aabb.max.z - aabb.min.z + 1) / 2 + aabb.min.z + 1
        }
    };
};

auto getHsaColor = [](const HardcodedSpawnAreaType& type) {
    switch (type) {
    case HardcodedSpawnAreaType::NetherFortress:
        return mce::Color::GREEN();
        break;
    case HardcodedSpawnAreaType::PillagerOutpost:
        return mce::Color::BLUE();
        break;
    case HardcodedSpawnAreaType::OceanMonument:
        return mce::Color::YELLOW();
        break;
    case HardcodedSpawnAreaType::WitchHut:
        return mce::Color::RED();
        break;
    default:
        return mce::Color::WHITE();
        break;
    }
};

} // namespace

namespace coral_fans::functions {

void HsaManager::drawHsa(LevelChunk::SpawningArea hsa) {
    // if not show: show particle
    coral_fans::mod().getLogger().warn("HsaManager::drawHsa");
    AABB* _aabb = (AABB*)&hsa.mUnkaa0f6a;
    if (this->mParticleMap.find(*_aabb) != this->mParticleMap.end()) return;
    ::HardcodedSpawnAreaType* _type = (::HardcodedSpawnAreaType*)&hsa.mUnkcb47a3;
    int                       dim   = *_type == HardcodedSpawnAreaType::NetherFortress ? 1 : 0;
    mce::Color                color = ::getHsaColor(*_type);
    auto                      aabb  = ::getSpawnAreaFromHSA(*_aabb);
    auto&                     mod   = coral_fans::mod();
    auto                      ids   = std::array{
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.min.x, aabb.min.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.min.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.min.y, aabb.max.z}, {aabb.max.x, aabb.min.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.max.x, aabb.min.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.min.x, aabb.max.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.max.x, aabb.max.y, aabb.min.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.max.y, aabb.min.z}, {aabb.min.x, aabb.max.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.max.y, aabb.min.z}, {aabb.max.x, aabb.max.y, aabb.min.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.max.x, aabb.min.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.max.y, aabb.max.z}, {aabb.min.x, aabb.min.y, aabb.max.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.max.x, aabb.max.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.min.z}, color),
        mod.getGeometryGroup()
            ->line(dim, {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.min.x, aabb.max.y, aabb.min.z}, mce::Color::WHITE())
    };
    this->mParticleMap[*_aabb] = mod.getGeometryGroup()->merge(ids);
}

void HsaManager::tick() {
    static int gt;
    if (gt == 0 && this->mShow) {
        auto level = ll::service::getLevel();
        if (level.has_value()) {
            // get players
            level->forEachPlayer([&](Player& player) {
                auto originChunkPos = utils::blockPosToChunkPos(((const Actor&)player).getFeetBlockPos());
                // chunks
                for (int i = -radius; i <= radius; ++i) {
                    for (int j = -radius; j <= radius; ++j) {
                        auto chunk = ((const Actor&)player)
                                         .getDimension()
                                         .getChunkSource()
                                         .getExistingChunk(ChunkPos{originChunkPos.x + i, originChunkPos.z + j});
                        if (chunk && chunk->isFullyLoaded()) {
                            // HSAs

                            std::vector<LevelChunk::SpawningArea>* _hsa =
                                (std::vector<LevelChunk::SpawningArea>*)&chunk->mSpawningAreas;
                            coral_fans::mod().getLogger().warn(
                                "HsaManager::chunk->isFullyLoaded() " + std::to_string(_hsa->size())
                            );
                            for (const auto hsa : *_hsa) {
                                coral_fans::mod().getLogger().warn("HsaManager::1");
                                this->drawHsa(hsa);
                            }
                        }
                    }
                }
                return true;
            });
        }
    }
    gt = (gt + 1) % 80;
}

void HsaManager::remove() {
    auto it = this->mParticleMap.begin();
    while (it != this->mParticleMap.end()) {
        coral_fans::mod().getGeometryGroup()->remove(it->second);
        this->mParticleMap.erase(it++);
    }
}

} // namespace coral_fans::functions