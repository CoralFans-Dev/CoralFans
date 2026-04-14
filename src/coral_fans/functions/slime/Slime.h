#pragma once

#include "bsci/GeometryGroup.h"

#include "mc/world/level/ChunkPos.h"

#include <unordered_map>

namespace coral_fans::functions {

class SlimeManager {
private:
    std::unordered_map<ChunkPos, std::pair<bsci::GeometryGroup::GeoId, int>> mParticleMap;

private:
    bool mShow;

    int tickCounter              = 0;
    int runtimeRemoveTickCounter = 1;

public:
    void setShow(bool);
    bool getShow();

public:
    void tick();

private:
    void draw();
    void remove();
    void runtimeRemove();

public:
    static SlimeManager& getInstance() {
        static SlimeManager instance;
        return instance;
    }
};

} // namespace coral_fans::functions