#pragma once

#include "mc/deps/nbt/CompoundTag.h"
#include "mc/entity/systems/LevelChunkTickingSystem.h"
#include "mc/network/packet/TextPacketType.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include <string>


namespace coral_fans::utils {

std::pair<std::string, bool> getNbtFromTag(CompoundTag&, std::string const&);

ChunkPos blockPosToChunkPos(BlockPos const& blockPos);

std::string removeMinecraftPrefix(std::string const& s);

void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time);

void sendInventorySwap(Player* player, int slot1, int slot2);

void segmentAndSendToPlayer(std::string const& text, Player* player, TextPacketType textType = TextPacketType::Raw);

void segmentAndSendToClients(std::string const& text, TextPacketType textType = TextPacketType::Raw);
} // namespace coral_fans::utils