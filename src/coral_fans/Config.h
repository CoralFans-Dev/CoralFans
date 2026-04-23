#pragma once

#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>
#include <vector>


namespace coral_fans::config {

struct CommandConfigStruct {
    bool                   enabled;
    CommandPermissionLevel permission;
    std::string            command;
};

struct CommandStruct {
    CommandConfigStruct tick       = {true, CommandPermissionLevel::GameDirectors, "tick"};
    CommandConfigStruct func       = {true, CommandPermissionLevel::GameDirectors, "func"};
    CommandConfigStruct self       = {true, CommandPermissionLevel::Any, "self"};
    CommandConfigStruct hsa        = {true, CommandPermissionLevel::Any, "hsa"};
    CommandConfigStruct counter    = {true, CommandPermissionLevel::GameDirectors, "counter"};
    CommandConfigStruct prof       = {true, CommandPermissionLevel::Any, "prof"};
    CommandConfigStruct slime      = {true, CommandPermissionLevel::Any, "slime"};
    CommandConfigStruct village    = {true, CommandPermissionLevel::Any, "village"};
    CommandConfigStruct rotate     = {true, CommandPermissionLevel::Any, "rotate"};
    CommandConfigStruct data       = {true, CommandPermissionLevel::Any, "data"};
    CommandConfigStruct cfhud      = {true, CommandPermissionLevel::Any, "cfhud"};
    CommandConfigStruct log        = {true, CommandPermissionLevel::Any, "log"};
    CommandConfigStruct calculate  = {true, CommandPermissionLevel::Any, "calculate"};
    CommandConfigStruct minerule   = {true, CommandPermissionLevel::GameDirectors, "minerule"};
    CommandConfigStruct freecamera = {true, CommandPermissionLevel::Any, "freecamera"};
    CommandConfigStruct noclip     = {true, CommandPermissionLevel::Any, "noclip"};
    CommandConfigStruct locate     = {true, CommandPermissionLevel::Any, "cflocate"};
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

#ifdef LL_PLAT_C
    struct keyBoard {
        bool                     enable;
        int                      keyCode     = 0;
        bool                     isDown      = true;
        std::string              description = "";
        bool                     intercept   = false;
        std::vector<std::string> actions;
    };
#endif
};

struct Locate {
    struct DuplicatableOreStruct {
        bool        enable             = true;
        std::string originPosColor     = "#10E436";
        std::string originPosTextColor = "#FFFFFF";
        std::string arrowColor         = "#FFFFFF";
    };
};

struct Config {
    int         version    = 7;
    std::string locateName = "zh_CN";

    CommandStruct command;

    struct {
        struct {
            int refreshInterval = 20;
        } hud{};

        struct {
            int         drawInterval       = 60;
            int         runtimeRemoveScale = 20;
            int         drawRadius         = 6;
            std::string hsaNorthWestColor  = "#FFFFFF";
            std::string hsaColor           = "#29ADFF";
            std::string structureColor     = "#10E436";
        } hsa{};

        struct {
            struct {
                int drawInterval       = 60;
                int runtimeRemoveScale = 20;
                int cacheRemoveScale   = 100;
                int drawRadius         = 6;
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string posColor       = "#FFFFFF";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } netherite{};
                struct {
                    bool        enable     = true;
                    std::string posColor   = "#10E436";
                    std::string textColor  = "#FFFFFF";
                    std::string arrowColor = "#FFFFFF";
                } netherSpring{};
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string boundColor     = "#FFFFFF";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } netherFire{};
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string boundColor     = "#FFFFFF";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } glowStone{};
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string boundColor     = "#FFFFFF";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } mushroom{};
                Locate::DuplicatableOreStruct netherGold{};
                Locate::DuplicatableOreStruct netherQuartz{};
                Locate::DuplicatableOreStruct netherMagma{};
                Locate::DuplicatableOreStruct netherGravel{};
                Locate::DuplicatableOreStruct netherBlackstone{};
                Locate::DuplicatableOreStruct netherSoulSand{};
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string boundColor     = "#FFFFFF";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } endIsland{};
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } chorusFlower{};
                struct {
                    bool        enable         = true;
                    std::string originPosColor = "#10E436";
                    std::string textColor      = "#FFFFFF";
                    std::string arrowColor     = "#FFFFFF";
                } endGateway{};
                struct {
                    std::string savedChunk   = "#3003D9";
                    std::string unsavedChunk = "#FFC825";
                } chunkSavedDebugInfo{};
            } duplicatable{};
        } locate{};

        struct {
            int         drawInterval       = 60;
            int         runtimeRemoveScale = 20;
            int         drawRadius         = 8;
            std::string slimeChunkColor    = "#10E436";
        } slime{};

        struct {
            int         drawInterval         = 10;
            std::string boundsColor          = "#FFFFFF";
            std::string raidBoundsColor      = "#10E436";
            std::string ironSpawnBoundsColor = "#3003D9";
            std::string centerColor          = "#FF0040";
            std::string poiBoundsColor       = "#FFA214";
            std::string bedBindColor         = "#DB3FFD";
            std::string ringBindColor        = "#FFEC27";
            std::string workBindColor        = "#5AC54F";
        } village{};
    } functions{};

    struct {
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
            {.enable      = true,
             .command     = "fc",
             .description = "freeCamera",
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

#ifdef LL_PLAT_C
        std::vector<Shortcut::keyBoard> keyBoards = {
            {.enable      = true,
             .keyCode     = 'F',
             .isDown      = true,
             .description = "freeCamera",
             .intercept   = false,
             .actions     = {"freecamera"}},
            {.enable      = true,
             .keyCode     = 'R',
             .isDown      = true,
             .description = "ratate",
             .intercept   = false,
             .actions     = {"rotate"}    },
            {.enable      = false,
             .keyCode     = 'N',
             .isDown      = true,
             .description = "noclip",
             .intercept   = false,
             .actions     = {"noclip"}    }
        };
#endif
    } shortcut{};
};

} // namespace coral_fans::config
