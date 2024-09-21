#pragma once

#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"

namespace coral_fans::functions {

std::string                  getBlockData(BlockPos, Block const&);
std::pair<std::string, bool> getBlockNbt(uint64, BlockSource&, BlockPos, std::string);
std::pair<std::string, bool> getEntityData(Actor*);
std::pair<std::string, bool> getEntityNbt(Actor*, std::string);
std::pair<std::string, bool> showRedstoneComponentsInfo(Dimension&, BlockPos&, uint64);
std::pair<std::string, bool> getItemNbt(ItemStack const&);

} // namespace coral_fans::functions