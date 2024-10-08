#pragma once

#include "bsci/GeometryGroup.h"

#include "mc/world/level/ChunkPos.h"

#include <unordered_map>

namespace coral_fans::functions {

class SlimeManager {
private:
    std::unordered_map<ChunkPos, bsci::GeometryGroup::GeoId> mParticleMap;
    bool                                                     mShow;

public:
    inline void setShow(bool enable) { this->mShow = enable; }
    void        tick();
    void        remove();
};

} // namespace coral_fans::functions