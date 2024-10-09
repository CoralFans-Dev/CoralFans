#pragma once

#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>
#include <unordered_set>
#include <vector>

namespace coral_fans::config {

struct CommandConfigStruct {
    bool                   enabled;
    CommandPermissionLevel permission;
};

struct CommandStruct {
    CommandConfigStruct tick    = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct func    = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct self    = {true, CommandPermissionLevel::Any};
    CommandConfigStruct hsa     = {true, CommandPermissionLevel::Any};
    CommandConfigStruct counter = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct prof    = {true, CommandPermissionLevel::Any};
    CommandConfigStruct slime   = {true, CommandPermissionLevel::Any};
    CommandConfigStruct village = {true, CommandPermissionLevel::Any};
    CommandConfigStruct rotate  = {true, CommandPermissionLevel::Any};
    CommandConfigStruct data    = {true, CommandPermissionLevel::Any};
    CommandConfigStruct cfhud   = {true, CommandPermissionLevel::Any};
    CommandConfigStruct sp      = {true, CommandPermissionLevel::Any};
};

struct Shortcut {
    bool                     enable;
    std::string              type;
    std::string              item;
    std::string              block;
    std::string              command;
    std::string              description;
    CommandPermissionLevel   permission;
    bool                     prevent;
    std::vector<std::string> actions;
};

enum class ListType : int { disabled, blacklist, whitelist };

struct SimPlayerStruct {
    std::string                     namePrefix      = "SIM-";
    std::string                     namePostfix     = "";
    unsigned long long              maxOnline       = 16;
    unsigned long long              maxOwn          = 4;
    unsigned long long              maxGroup        = 8;
    unsigned long long              maxSpawnCount   = 128;
    CommandPermissionLevel          adminPermission = CommandPermissionLevel::GameDirectors;
    ListType                        listType        = ListType::disabled;
    std::unordered_set<std::string> list;
    std::string                     luaPreload = "";
};

struct Config {
    int         version    = 2;
    std::string locateName = "zh_CN";

    int cfhudRefreshTime = 20;

    SimPlayerStruct simPlayer;

    CommandStruct         command;
    std::vector<Shortcut> shortcuts = {
  /* hoppercounter */
        {.enable = true, .type = "useon", .item = "cactus", .block = "white_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "orange_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "magenta_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "light_blue_concrete", .actions = {"counter print"}
        },
        {.enable = true, .type = "useon", .item = "cactus", .block = "yellow_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "lime_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "pink_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "gray_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "light_gray_concrete", .actions = {"counter print"}
        },
        {.enable = true, .type = "useon", .item = "cactus", .block = "cyan_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "purple_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "blue_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "brown_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "green_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "red_concrete", .actions = {"counter print"}},
        {.enable = true, .type = "useon", .item = "cactus", .block = "black_concrete", .actions = {"counter print"}},
 /* blockrotate */
        {.enable = true, .type = "use", .item = "cactus", .prevent = true, .actions = {"rotate"}},
        {.enable      = true,
         .type        = "command",
         .command     = "r",
         .description = "rotate",
         .permission  = CommandPermissionLevel::Any,
         .actions     = {"rotate"}},
 /* fastcommand */
        {.enable      = false,
         .type        = "command",
         .command     = "c",
         .description = "creative",
         .permission  = CommandPermissionLevel::GameDirectors,
         .actions     = {"gamemode creative"}},
        {.enable      = false,
         .type        = "command",
         .command     = "s",
         .description = "spectator",
         .permission  = CommandPermissionLevel::GameDirectors,
         .actions     = {"gamemode spectator"}},
        {.enable      = false,
         .type        = "command",
         .command     = "q",
         .description = "suicide",
         .permission  = CommandPermissionLevel::GameDirectors,
         .actions     = {"gamemode adventure", "kill", "gamemode creative"}}
    };
};

} // namespace coral_fans::config
