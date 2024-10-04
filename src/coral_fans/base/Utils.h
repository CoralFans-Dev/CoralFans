#pragma once

#include "mc/deps/core/mce/Color.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"

#include <memory>
#include <optional>
#include <string>

namespace coral_fans::utils {


BlockPos facingToBlockPos(int facing);

ChunkPos blockPosToChunkPos(BlockPos const& blockPos);

std::string removeMinecraftPrefix(std::string const& s);

void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time);

void swapItemInContainer(Player* player, int slot1, int slot2);

std::optional<std::pair<CompoundTag, CompoundTag>> getItemFromShulkerBox(
    std::unique_ptr<CompoundTag> tag,
    ItemStack const&             itemStack,
    bool                         replace  = false,
    int                          minCount = 0
);

} // namespace coral_fans::utils