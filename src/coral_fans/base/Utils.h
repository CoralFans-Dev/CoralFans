#pragma once

#include "coral_fans/base/Mod.h"

#include "mc/deps/core/mce/Color.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"

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

inline ChunkPos blockPosToChunkPos(BlockPos const& blockPos) {
    return ChunkPos{
        (blockPos.x < 0 ? blockPos.x - 15 : blockPos.x) / 16,
        (blockPos.z < 0 ? blockPos.z - 15 : blockPos.z) / 16
    };
}

inline std::string removeMinecraftPrefix(std::string const& s) { return s.find("minecraft:") == 0 ? s.substr(10) : s; }

inline void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time) {
    auto& mod = coral_fans::mod();
    auto  s   = mod.getGeometryGroup()->box(dimid, {blockPos, blockPos + BlockPos::ONE}, color);
    mod.getScheduler().add(time, [&, s] {
        mod.getGeometryGroup()->remove(s);
        return false;
    });
}

} // namespace coral_fans::utils