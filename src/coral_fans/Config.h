#pragma once

#include <string>
#include <vector>

namespace coral_fans {

struct CommandConfigStruct {
    bool        enabled;
    std::string permission;
};

struct CommandStruct {
    CommandConfigStruct tick    = {true, "GameDirectors"};
    CommandConfigStruct func    = {true, "GameDirectors"};
    CommandConfigStruct self    = {true, "Any"};
    CommandConfigStruct hsa     = {true, "Any"};
    CommandConfigStruct counter = {true, "GameDirectors"};
    CommandConfigStruct prof    = {true, "Any"};
    CommandConfigStruct slime   = {true, "Any"};
    CommandConfigStruct village = {true, "Any"};
};

struct Shortcut {
    bool                     enable;
    std::string              type;
    std::string              item;
    std::string              block;
    std::string              command;
    std::string              description;
    std::string              permission;
    bool                     prevent;
    std::vector<std::string> actions;
};

struct Config {
    int         version    = 1;
    std::string locateName = "zh_CN";

    CommandStruct         command;
    std::vector<Shortcut> shortcuts = {
        {true,  "useon",   "cactus", "white_concrete",      "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "orange_concrete",     "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "magenta_concrete",    "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "light_blue_concrete", "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "yellow_concrete",     "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "lime_concrete",       "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "pink_concrete",       "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "gray_concrete",       "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "light_gray_concrete", "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "cyan_concrete",       "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "purple_concrete",     "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "blue_concrete",       "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "brown_concrete",      "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "green_concrete",      "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "red_concrete",        "",  "",          "",              false, {"counter print"}                                  },
        {true,  "useon",   "cactus", "black_concrete",      "",  "",          "",              false, {"counter print"}                                  },
        {false, "command", "",       "",                    "c", "creative",  "GameDirectors", false, {"gamemode creative"}                              },
        {false, "command", "",       "",                    "s", "spectator", "GameDirectors", false, {"gamemode spectator"}                             },
        {false,
         "command",        "",
         "",                                                "q",
         "suicide",                                                           "GameDirectors",
         false,                                                                                       {"gamemode adventure", "kill", "gamemode creative"}}
    };
};

} // namespace coral_fans
