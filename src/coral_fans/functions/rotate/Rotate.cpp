#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/chunk/LevelChunk.h"


#include <string>
#include <unordered_map>

namespace {

std::unordered_map<std::string, std::string> leverDirection = {
    {"down_east_west",   "east"            },
    {"east",             "west"            },
    {"west",             "south"           },
    {"south",            "north"           },
    {"north",            "up_north_south"  },
    {"up_north_south",   "up_east_west"    },
    {"up_east_west",     "down_north_south"},
    {"down_north_south", "down_east_west"  }
};

std::unordered_map<std::string, std::string> minecraftFacingDirection = {
    {"down",  "up"   },
    {"up",    "north"},
    {"north", "south"},
    {"south", "west" },
    {"west",  "east" },
    {"east",  "down" }
};

std::unordered_map<int, int> facingDirection = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 4},
    {4, 5},
    {5, 0}
};

std::unordered_map<std::string, std::string> torchFacingDirection = {
    {"unknown", "west"   },
    {"west",    "east"   },
    {"east",    "north"  },
    {"north",   "south"  },
    {"south",   "top"    },
    {"top",     "unknown"}
};

std::unordered_map<std::string, std::string> minecraftCardinalDirection = {
    {"south", "west" },
    {"west",  "north"},
    {"north", "east" },
    {"east",  "south"}
};

std::unordered_map<int, int> groundSignDirection = {
    {0,  1 },
    {1,  2 },
    {2,  3 },
    {3,  4 },
    {4,  5 },
    {5,  6 },
    {6,  7 },
    {7,  8 },
    {8,  9 },
    {9,  10},
    {10, 11},
    {11, 12},
    {12, 13},
    {13, 14},
    {14, 15},
    {15, 0 }
};

std::unordered_map<std::string, std::string> pillarAxis = {
    {"y", "x"},
    {"x", "z"},
    {"z", "y"}
};

std::unordered_map<std::string, std::string> portalAxis = {
    {"unknown", "x"      },
    {"x",       "z"      },
    {"z",       "unknown"}
};

std::unordered_map<std::string, std::string> minecraftVerticalHalf = {
    {"bottom", "top"   },
    {"top",    "bottom"}
};

std::unordered_map<int, int> numDirection = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0}
};

std::unordered_map<std::string, std::string> minecraftBlockFace = {
    {"down",  "up"   },
    {"up",    "north"},
    {"north", "south"},
    {"south", "west" },
    {"west",  "east" },
    {"east",  "down" }
};

std::unordered_map<int, int> railDirection = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 4},
    {4, 5},
    {5, 0}
};

std::unordered_map<std::string, std::string> orientation = {
    {"down_east",  "down_north"},
    {"down_north", "down_south"},
    {"down_south", "down_west" },
    {"down_west",  "up_east"   },
    {"up_east",    "up_north"  },
    {"up_north",   "up_south"  },
    {"up_south",   "up_west"   },
    {"up_west",    "west_up"   },
    {"west_up",    "east_up"   },
    {"east_up",    "north_up"  },
    {"north_up",   "south_up"  },
    {"south_up",   "down_east" }
};

std::unordered_map<int, int> coralDirection = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0}
};

std::unordered_map<int, int> weirdoDirection = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0}
};

} // namespace

namespace coral_fans::functions {

void rotateBlock(Player* player, BlockPos blockPos) {
    auto&       blockSource = player->getDimensionBlockSource();
    const auto& block       = blockSource.getBlock(blockPos);
    const auto  rawTypeName = block.getTypeName();
    const auto  typeName    = utils::removeMinecraftPrefix(rawTypeName);
    auto        nbtTag      = *block.mSerializationId;
    if (!nbtTag.contains("states")) return;
    auto states = nbtTag["states"];
    if (states.contains("lever_direction")) {
        auto direction = std ::string(states["lever_direction"].get<StringTag>());
        auto next      = ::leverDirection.find(direction);
        if (next == ::leverDirection.end()) return;
        auto newTag                         = CompoundTag(nbtTag);
        newTag["states"]["lever_direction"] = StringTag(next->second);
        auto newBlock                       = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("torch_facing_direction")) {
        auto direction = std ::string(states["torch_facing_direction"].get<StringTag>());
        auto next      = ::torchFacingDirection.find(direction);
        if (next == ::torchFacingDirection.end()) return;
        auto newTag                                = CompoundTag(nbtTag);
        newTag["states"]["torch_facing_direction"] = StringTag(next->second);
        auto newBlock                              = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("ground_sign_direction")) {
        auto direction = int(states["ground_sign_direction"].get<IntTag>());
        auto next      = ::groundSignDirection.find(direction);
        if (next == ::groundSignDirection.end()) return;
        auto newTag                               = CompoundTag(nbtTag);
        newTag["states"]["ground_sign_direction"] = IntTag(next->second);
        auto newBlock                             = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("minecraft:cardinal_direction")) {
        auto direction = std ::string(states["minecraft:cardinal_direction"].get<StringTag>());
        auto next      = ::minecraftCardinalDirection.find(direction);
        if (next == ::minecraftCardinalDirection.end()) return;
        auto newTag                                      = CompoundTag(nbtTag);
        newTag["states"]["minecraft:cardinal_direction"] = StringTag(next->second);
        auto newBlock                                    = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("minecraft:facing_direction")) {
        auto direction = std ::string(states["minecraft:facing_direction"].get<StringTag>());
        auto next      = ::minecraftFacingDirection.find(direction);
        if (next == ::minecraftFacingDirection.end()) return;
        auto newTag                                    = CompoundTag(nbtTag);
        newTag["states"]["minecraft:facing_direction"] = StringTag(next->second);
        auto newBlock                                  = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("facing_direction")) {
        auto direction = int(states["facing_direction"].get<IntTag>());
        auto next      = ::facingDirection.find(direction);
        if (next == ::facingDirection.end()) return;
        auto newTag                          = CompoundTag(nbtTag);
        newTag["states"]["facing_direction"] = IntTag(next->second);
        auto newBlock                        = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("pillar_axis")) {
        auto direction = std ::string(states["pillar_axis"].get<StringTag>());
        auto next      = ::pillarAxis.find(direction);
        if (next == ::pillarAxis.end()) return;
        auto newTag                     = CompoundTag(nbtTag);
        newTag["states"]["pillar_axis"] = StringTag(next->second);
        auto newBlock                   = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("portal_axis")) {
        auto direction = std ::string(states["portal_axis"].get<StringTag>());
        auto next      = ::portalAxis.find(direction);
        if (next == ::portalAxis.end()) return;
        auto newTag                     = CompoundTag(nbtTag);
        newTag["states"]["portal_axis"] = StringTag(next->second);
        auto newBlock                   = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("minecraft:vertical_half")) {
        auto direction = std ::string(states["minecraft:vertical_half"].get<StringTag>());
        auto next      = ::minecraftVerticalHalf.find(direction);
        if (next == ::minecraftVerticalHalf.end()) return;
        auto newTag                                 = CompoundTag(nbtTag);
        newTag["states"]["minecraft:vertical_half"] = StringTag(next->second);
        auto newBlock                               = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("direction")) {
        auto direction = int(states["direction"].get<IntTag>());
        auto next      = ::numDirection.find(direction);
        if (next == ::numDirection.end()) return;
        auto newTag                   = CompoundTag(nbtTag);
        newTag["states"]["direction"] = IntTag(next->second);
        auto newBlock                 = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("minecraft:block_face")) {
        auto direction = std ::string(states["minecraft:block_face"].get<StringTag>());
        auto next      = ::minecraftBlockFace.find(direction);
        if (next == ::minecraftBlockFace.end()) return;
        auto newTag                              = CompoundTag(nbtTag);
        newTag["states"]["minecraft:block_face"] = StringTag(next->second);
        auto newBlock                            = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("rail_direction")) {
        auto direction = int(states["rail_direction"].get<IntTag>());
        auto next      = ::railDirection.find(direction);
        if (next == ::railDirection.end()) return;
        auto newTag                        = CompoundTag(nbtTag);
        newTag["states"]["rail_direction"] = IntTag(next->second);
        auto newBlock                      = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("orientation")) {
        auto direction = std ::string(states["orientation"].get<StringTag>());
        auto next      = ::orientation.find(direction);
        if (next == ::orientation.end()) return;
        auto newTag                     = CompoundTag(nbtTag);
        newTag["states"]["orientation"] = StringTag(next->second);
        auto newBlock                   = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("coral_direction")) {
        auto direction = int(states["coral_direction"].get<IntTag>());
        auto next      = ::coralDirection.find(direction);
        if (next == ::coralDirection.end()) return;
        auto newTag                         = CompoundTag(nbtTag);
        newTag["states"]["coral_direction"] = IntTag(next->second);
        auto newBlock                       = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    } else if (states.contains("weirdo_direction")) {
        auto direction = int(states["weirdo_direction"].get<IntTag>());
        auto next      = ::weirdoDirection.find(direction);
        if (next == ::weirdoDirection.end()) return;
        auto newTag                          = CompoundTag(nbtTag);
        newTag["states"]["weirdo_direction"] = IntTag(next->second);
        auto newBlock                        = Block ::tryGetFromRegistry(newTag);
        if (!newBlock) return;
        BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) {
            std ::unique_ptr<CompoundTag> blockEntityTag = std ::make_unique<CompoundTag>();
            blockEntity->save(*blockEntityTag, *SaveContextFactory ::createCloneSaveContext());
            blockSource.getChunkAt(blockPos)->removeBlockEntity(blockPos);
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, BlockActor ::create(*blockEntityTag), nullptr, nullptr);
            return;
        } else {
            blockSource.setBlock(blockPos, Block ::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);
            blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);
            return;
        }
    }
}

} // namespace coral_fans::functions