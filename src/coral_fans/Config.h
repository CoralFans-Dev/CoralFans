#pragma once

#include <string>

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
};

struct Config {
    int         version    = 1;
    std::string locateName = "zh_CN";

    CommandStruct command;
};

} // namespace coral_fans
