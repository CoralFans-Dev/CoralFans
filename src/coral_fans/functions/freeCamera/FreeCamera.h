// from https://github.com/GroupMountain/FreeCamera
#pragma once

#include "mc/world/actor/player/Player.h"


#include <unordered_set>
namespace coral_fans::functions {
class FreeCameraManager {
private:
    FreeCameraManager() = default;

public:
    std::unordered_set<std::string>         FreeCamList;
    [[nodiscard]] static FreeCameraManager& getInstance() {
        static FreeCameraManager instance;
        return instance;
    }
    static void DisableFreeCamera(Player* pl);
    static void EnableFreeCamera(Player* pl);
    static void freecameraHook(bool enable);
};
} // namespace coral_fans::functions