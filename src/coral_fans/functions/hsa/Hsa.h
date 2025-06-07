#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"

#include <unordered_map>
#include <vector>

namespace coral_fans::functions {

class HsaManager {
private:
    std::unordered_map<ChunkPos, std::vector<bsci::GeometryGroup::GeoId>> mParticleMap[3];
    std::unordered_map<ChunkPos, bool>                                    mChunkLoaded[3];
    bool                                                                  mShow;

public:
    void        drawHsa();
    inline void setShow(bool enable) { this->mShow = enable; }
    void        tick();
    void        remove();
    void        runtimeRemove();
};

} // namespace coral_fans::functions