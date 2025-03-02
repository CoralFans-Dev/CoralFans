#pragma once

#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"


namespace coral_fans::functions {

std::string                  getBlockData(BlockSource&, BlockPos);
std::pair<std::string, bool> getBlockNbt(uint64, BlockSource&, BlockPos, std::string);
std::pair<std::string, bool> getEntityData(Actor*);
std::pair<std::string, bool> getEntityNbt(Actor*, std::string);
std::pair<std::string, bool> showRedstoneComponentsInfo(Dimension&, BlockPos&, uint64);
std::pair<std::string, bool> getItemNbt(ItemStack const&, std::string);
void                         highlightBlockEntity(Player*, int, int);

} // namespace coral_fans::functions