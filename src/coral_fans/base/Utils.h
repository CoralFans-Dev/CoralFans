#pragma once

#include "mc/world/level/BlockPos.h"

#include <string>

namespace coral_fans::utils {


inline BlockPos facingToBlockPos(int facing) {
    switch (facing) {
    case 0:
        return BlockPos{0, -1, 0};
        break;
    case 1:
        return BlockPos{0, +1, 0};
        break;
    case 2:
        return BlockPos{0, 0, -1};
        break;
    case 3:
        return BlockPos{0, 0, +1};
        break;
    case 4:
        return BlockPos{-1, 0, 0};
        break;
    case 5:
        return BlockPos{+1, 0, 0};
        break;
    default:
        return BlockPos{0, 0, 0};
        break;
    }
};


inline std::string removeMinecraftPrefix(const std::string& s) { return s.find("minecraft:") == 0 ? s.substr(10) : s; }

} // namespace coral_fans::utils