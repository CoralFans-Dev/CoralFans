#pragma once

#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"

namespace coral_fans::functions {

std::string                  getBlockData(BlockPos, Block const&);
std::pair<std::string, bool> getBlockNbt(uint64, BlockSource&, BlockPos, std::string);
std::pair<std::string, bool> getEntityData(Actor*);
std::pair<std::string, bool> getEntityNbt(Actor*, std::string);

} // namespace coral_fans::functions