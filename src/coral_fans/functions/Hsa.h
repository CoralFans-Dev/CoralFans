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

public:
    void drawHsa(LevelChunk::HardcodedSpawningArea);
    void show(bool);
};

} // namespace coral_fans::functions