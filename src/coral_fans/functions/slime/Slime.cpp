#include "coral_fans/functions/slime/Slime.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"

#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"

namespace {
static const int radius = 5;
}

namespace coral_fans::functions {

void SlimeManager::tick() {
    static int gt = 0;
    if (gt == 0 && this->mShow) {
        auto level = ll::service::getLevel();
        if (level.has_value()) {
            // get players
            level->forEachPlayer([&](Player& player) {
                auto originChunkPos = utils::blockPosToChunkPos(player.getFeetBlockPos());
                // chunks
                for (int i = -radius; i <= radius; ++i) {
                    for (int j = -radius; j <= radius; ++j) {
                        auto         seed = ((originChunkPos.x + i) * 0x1f1f1f1fu) ^ (uint32_t)(originChunkPos.z + j);
                        std::mt19937 mt(seed);
                        if (mt() % 10 == 0
                            && this->mParticleMap.find(ChunkPos{originChunkPos.x + i, originChunkPos.z + j})
                                   == this->mParticleMap.end())
                            this->mParticleMap[ChunkPos{
                                originChunkPos.x + i,
                                originChunkPos.z + j
                            }] =
                                coral_fans::mod().getGeometryGroup()->box(
                                    0,
                                    BoundingBox{
                                        {(originChunkPos.x + i) * 16, -64, (originChunkPos.z + j) * 16},
                                        {(originChunkPos.x + i) * 16 + 15, 320, (originChunkPos.z + j) * 16 + 15}
                                    },
                                    mce::Color::GREEN()
                                );
                    }
                }
                return true;
            });
        }
    }
    gt = (gt + 1) % 80;
}

void SlimeManager::remove() {
    auto it = this->mParticleMap.begin();
    while (it != this->mParticleMap.end()) {
        coral_fans::mod().getGeometryGroup()->remove(it->second);
        this->mParticleMap.erase(it++);
    }
}

} // namespace coral_fans::functions