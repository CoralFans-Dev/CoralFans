#pragma once

// #include "coral_fans/functions/shortcuts/Shortcuts.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>
#include <vector>


namespace coral_fans::config {

struct CommandConfigStruct {
    bool                   enabled;
    CommandPermissionLevel permission;
};

struct CommandStruct {
    CommandConfigStruct tick       = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct func       = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct self       = {true, CommandPermissionLevel::Any};
    CommandConfigStruct hsa        = {true, CommandPermissionLevel::Any};
    CommandConfigStruct counter    = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct prof       = {true, CommandPermissionLevel::Any};
    CommandConfigStruct slime      = {true, CommandPermissionLevel::Any};
    CommandConfigStruct village    = {true, CommandPermissionLevel::Any};
    CommandConfigStruct rotate     = {true, CommandPermissionLevel::Any};
    CommandConfigStruct data       = {true, CommandPermissionLevel::Any};
    CommandConfigStruct cfhud      = {true, CommandPermissionLevel::Any};
    CommandConfigStruct log        = {true, CommandPermissionLevel::Any};
    CommandConfigStruct calculate  = {true, CommandPermissionLevel::Any};
    CommandConfigStruct minerule   = {true, CommandPermissionLevel::GameDirectors};
    CommandConfigStruct freecamera = {true, CommandPermissionLevel::Any};
    CommandConfigStruct noclip     = {true, CommandPermissionLevel::Any};
};

struct Shortcut {
    struct UseOn {
        bool                     enable;
        std::string              item;
        std::string              block     = "";
        bool                     intercept = false;
        std::vector<std::string> actions;
    };

    struct Use {
        bool                     enable;
        std::string              item;
        bool                     intercept = false;
        std::vector<std::string> actions;
    };

    struct Destroy {
        bool                     enable;
        std::string              item;
        bool                     intercept = false;
        std::vector<std::string> actions;
    };

    struct Command {
        bool                     enable;
        std::string              command     = "";
        std::string              description = "";
        CommandPermissionLevel   permission  = CommandPermissionLevel::Any;
        std::vector<std::string> actions;
    };
};

struct Config {
    int         version    = 5;
    std::string locateName = "zh_CN";

    int cfhudRefreshTime = 20;

    CommandStruct command;


    std::vector<Shortcut::UseOn> useons = {
        /* hoppercounter */
        {.enable = true, .item = "cactus", .block = "white_concrete",      .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "orange_concrete",     .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "magenta_concrete",    .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "light_blue_concrete", .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "yellow_concrete",     .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "lime_concrete",       .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "pink_concrete",       .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "gray_concrete",       .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "light_gray_concrete", .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "cyan_concrete",       .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "purple_concrete",     .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "blue_concrete",       .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "brown_concrete",      .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "green_concrete",      .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "red_concrete",        .actions = {"counter print"}},
        {.enable = true, .item = "cactus", .block = "black_concrete",      .actions = {"counter print"}}
    };

    std::vector<Shortcut::Use> uses = {
        /* blockrotate */
        {.enable = true, .item = "cactus", .intercept = true, .actions = {"rotate"}}
    };

    std::vector<Shortcut::Destroy> destroys = {};

    std::vector<Shortcut::Command> commands = {
        {.enable      = true,
         .command     = "r",
         .description = "rotate",
         .permission  = CommandPermissionLevel::Any,
         .actions     = {"rotate"}                                         },
        /* fastcommand */
        {.enable      = false,
         .command     = "c",
         .description = "creative",
         .permission  = CommandPermissionLevel::GameDirectors,
         .actions     = {"gamemode creative"}                              },
        {.enable      = false,
         .command     = "s",
         .description = "spectator",
         .permission  = CommandPermissionLevel::GameDirectors,
         .actions     = {"gamemode spectator"}                             },
        {.enable      = false,
         .command     = "q",
         .description = "suicide",
         .permission  = CommandPermissionLevel::GameDirectors,
         .actions     = {"gamemode adventure", "kill", "gamemode creative"}}
    };
};

} // namespace coral_fans::config
