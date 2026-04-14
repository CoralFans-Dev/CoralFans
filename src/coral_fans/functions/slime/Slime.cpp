#include "coral_fans/functions/slime/Slime.h"
#include "coral_fans/CoralFans.h"
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
void SlimeManager::setShow(bool show) {
    if (this->mShow == show) return;
    this->mShow = show;
    if (show) {
        this->tickCounter              = 0;
        this->runtimeRemoveTickCounter = 1;
    } else this->remove();
}

bool SlimeManager::getShow() { return this->mShow; }

void SlimeManager::draw() {
    auto level = ll::service::getLevel();
    if (!level.has_value()) [[unlikely]]
        return;
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    level->forEachPlayer([&](Player& player) {
        auto originChunkPos = utils::blockPosToChunkPos(player.getFeetBlockPos());
        for (int i = -radius; i <= radius; ++i) {
            int maxJ = radius - abs(i);
            for (int j = -maxJ; j <= maxJ; ++j) {
                auto         seed = ((originChunkPos.x + i) * 0x1f1f1f1fu) ^ (uint32_t)(originChunkPos.z + j);
                std::mt19937 mt(seed);
                if (mt() % 10 == 0) {
                    auto [it, inserted] =
                        this->mParticleMap.try_emplace(ChunkPos(originChunkPos.x + i, originChunkPos.z + j));
                    if (inserted) {
                        it->second.first = geometryGroup->box(
                            0,
                            AABB{
                                {(originChunkPos.x + i) * 16 + 0.1,  -64, (originChunkPos.z + j) * 16 + 0.1 },
                                {(originChunkPos.x + i) * 16 + 15.9, 320, (originChunkPos.z + j) * 16 + 15.9}
                        },
                            mce::Color::GREEN()
                        );
                    }
                    it->second.second = 0;
                }
            }
        }
        return true;
    });
}

void SlimeManager::tick() {
    if (!this->mShow) return;
    if (!this->tickCounter) {
        this->draw();
        if (!this->runtimeRemoveTickCounter) {
            this->runtimeRemove();
        }
        static int removeInterval =
            std::max(1, CoralFans::getInstance().getConfig().functions.slime.runtimeRemoveScale);
        this->runtimeRemoveTickCounter = (this->runtimeRemoveTickCounter + 1) % removeInterval;
    }
    static int interval = std::max(1, CoralFans::getInstance().getConfig().functions.slime.drawInterval);
    this->tickCounter   = (this->tickCounter + 1) % interval;
}

void SlimeManager::remove() {
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    for (auto& [_, data] : this->mParticleMap) {
        geometryGroup->remove(data.first);
    }
    this->mParticleMap.clear();
}

void SlimeManager::runtimeRemove() {
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    auto& slimeConfig   = CoralFans::getInstance().getConfig().functions.slime;
    std::erase_if(this->mParticleMap, [&geometryGroup, &slimeConfig](auto& data) {
        if (data.second.second == slimeConfig.runtimeRemoveScale) {
            geometryGroup->remove(data.second.first);
            return true;
        }
        return false;
    });
}
} // namespace coral_fans::functions