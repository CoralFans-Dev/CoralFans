#include "PortalManager.h"
#include "coral_fans/CoralFans.h"


#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/PortalForcer.h"
#include "mc/world/level/PortalRecord.h"


#include <algorithm>
#include <cstdlib>
#include <utility>
#include <vector>


namespace coral_fans::functions::locate {

std::vector<PortalManager::PortalInfo> PortalManager::getSortedPortals() {
    std::vector<PortalInfo> portals;
    auto                    level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return portals;

    auto& forcer = level->getPortalForcer();
    for (auto& [dimType, records] : *forcer.mPortalRecords) {
        for (auto& record : records) {
            portals.emplace_back(static_cast<int>(dimType), record.mBaseBlockPos.get());
        }
    }
    std::sort(portals.begin(), portals.end(), [](PortalInfo const& a, PortalInfo const& b) {
        int sumA = std::abs(a.pos.x) + std::abs(a.pos.z);
        int sumB = std::abs(b.pos.x) + std::abs(b.pos.z);
        if (sumA != sumB) return sumA < sumB;
        if (a.dimId != b.dimId) return a.dimId < b.dimId;
        if (a.pos.x != b.pos.x) return a.pos.x < b.pos.x;
        if (a.pos.z != b.pos.z) return a.pos.z < b.pos.z;
        return a.pos.y < b.pos.y;
    });
    return portals;
}

void PortalManager::setShow(bool show) {
    if (this->mShow == show) return;
    this->mShow = show;
    if (show) {
        this->tickCounter = 0;
    } else {
        this->remove();
        this->mDrawnPortals.clear();
    }
}

bool PortalManager::getShow() const { return this->mShow; }

void PortalManager::clear() { this->setShow(false); }

void PortalManager::draw() {
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;

    static int radius = std::max(0, CoralFans::getInstance().getConfig().functions.locate.portal.radius);

    std::vector<std::pair<int, ChunkPos>> playerChunkPoses;
    level->forEachPlayer([&playerChunkPoses](Player& player) {
        playerChunkPoses.emplace_back(static_cast<int>(player.getDimensionId()), ChunkPos(player.getFeetBlockPos()));
        return true;
    });

    std::vector<PortalInfo> visiblePortals;
    for (auto& info : getSortedPortals()) {
        ChunkPos portalChunkPos(info.pos);
        for (auto& [dimId, chunkPos] : playerChunkPoses) {
            if (dimId != info.dimId) continue;
            if (std::abs(portalChunkPos.x - chunkPos.x) <= radius
                && std::abs(portalChunkPos.z - chunkPos.z) <= radius) {
                visiblePortals.emplace_back(info);
                break;
            }
        }
    }
    if (visiblePortals == this->mDrawnPortals) return;

    this->remove();
    this->mDrawnPortals = std::move(visiblePortals);

    auto&              geometryGroup = CoralFans::getInstance().getGeometryGroup();
    static std::string colorStr      = CoralFans::getInstance().getConfig().functions.locate.portal.color;
    mce::Color         color(colorStr);

    this->mGeoIds.reserve(this->mDrawnPortals.size());
    for (auto& info : this->mDrawnPortals) {
        this->mGeoIds.emplace_back(geometryGroup->box(info.dimId, AABB(info.pos, info.pos + BlockPos(1, 1, 1)), color));
    }
}

void PortalManager::remove() {
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    for (auto& geoId : this->mGeoIds) {
        geometryGroup->remove(geoId);
    }
    this->mGeoIds.clear();
}

void PortalManager::tick() {
    if (!this->mShow) return;
    if (!this->tickCounter) {
        this->draw();
    }
    static int interval = std::max(1, CoralFans::getInstance().getConfig().functions.locate.portal.drawInterval);
    this->tickCounter   = (this->tickCounter + 1) % interval;
}

} // namespace coral_fans::functions::locate
