#pragma once

#include "mc/nbt/CompoundTag.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include <string>

namespace coral_fans::utils {

std::pair<std::string, bool> getNbtFromTag(CompoundTag const, std::string const&);

ChunkPos blockPosToChunkPos(BlockPos const& blockPos);

std::string removeMinecraftPrefix(std::string const& s);

void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time);

void swapItemInContainer(Player* player, int slot1, int slot2);

} // namespace coral_fans::utils