#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"

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
    auto        nbtTag      = block.getSerializationId();
    if (!nbtTag.contains("states")) return;
    auto states = nbtTag["states"];
    if (states.contains("lever_direction")) {
        ROTATE_BLOCK("lever_direction", ::leverDirection, std::string, StringTag)
    } else if (states.contains("torch_facing_direction")) {
        ROTATE_BLOCK("torch_facing_direction", ::torchFacingDirection, std::string, StringTag)
    } else if (states.contains("ground_sign_direction")) {
        ROTATE_BLOCK("ground_sign_direction", ::groundSignDirection, int, IntTag)
    } else if (states.contains("minecraft:cardinal_direction")) {
        ROTATE_BLOCK("minecraft:cardinal_direction", ::minecraftCardinalDirection, std::string, StringTag)
    } else if (states.contains("minecraft:facing_direction")) {
        ROTATE_BLOCK("minecraft:facing_direction", ::minecraftFacingDirection, std::string, StringTag)
    } else if (states.contains("facing_direction")) {
        ROTATE_BLOCK("facing_direction", ::facingDirection, int, IntTag)
    } else if (states.contains("pillar_axis")) {
        ROTATE_BLOCK("pillar_axis", ::pillarAxis, std::string, StringTag)
    } else if (states.contains("portal_axis")) {
        ROTATE_BLOCK("portal_axis", ::portalAxis, std::string, StringTag)
    } else if (states.contains("minecraft:vertical_half")) {
        ROTATE_BLOCK("minecraft:vertical_half", ::minecraftVerticalHalf, std::string, StringTag)
    } else if (states.contains("direction")) {
        ROTATE_BLOCK("direction", ::numDirection, int, IntTag)
    } else if (states.contains("minecraft:block_face")) {
        ROTATE_BLOCK("minecraft:block_face", ::minecraftBlockFace, std::string, StringTag)
    } else if (states.contains("rail_direction")) {
        ROTATE_BLOCK("rail_direction", ::railDirection, int, IntTag)
    } else if (states.contains("orientation")) {
        ROTATE_BLOCK("orientation", ::orientation, std::string, StringTag)
    } else if (states.contains("coral_direction")) {
        ROTATE_BLOCK("coral_direction", ::coralDirection, int, IntTag)
    } else if (states.contains("weirdo_direction")) {
        ROTATE_BLOCK("weirdo_direction", ::weirdoDirection, int, IntTag)
    }
}

} // namespace coral_fans::functions