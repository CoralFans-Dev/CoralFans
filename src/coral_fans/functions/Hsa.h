#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/phys/AABB.h"

#include <unordered_map>

namespace coral_fans::functions {

class HsaManager {
private:
    std::unordered_map<AABB, bsci::GeometryGroup::GeoId> mParticleMap;
    bool                                                 mShow;

public:
    void        drawHsa(LevelChunk::SpawningArea);
    inline void setShow(bool enable) { this->mShow = enable; }
    void        tick();
    void        remove();
};

} // namespace coral_fans::functions