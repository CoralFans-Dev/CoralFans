#pragma once

#include "mc/deps/core/mce/UUID.h"
#include "mc/server/SimulatedPlayer.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace coral_fans::functions {

class SimPlayerManager {
private:
    struct SimPlayerInfo {
        std::string                     name;
        std::unordered_set<std::string> groups;
        SimulatedPlayer*                simPlayer;
    };
    std::unordered_map<mce::UUID, std::unordered_set<SimPlayerInfo>> mSimPlayerMap;
    int                                                              mOnlineCount = 0;
};

} // namespace coral_fans::functions