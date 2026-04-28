#pragma once

#include "mc/world/actor/player/Player.h"
#include <string>
#include <unordered_set>

namespace coral_fans::functions {

class NoclipManager {
private:
    std::unordered_set<std::string> handlingList;

private:
    NoclipManager() = default;

public:
    NoclipManager(const NoclipManager&)            = delete;
    NoclipManager& operator=(const NoclipManager&) = delete;

    [[nodiscard]] static NoclipManager& getInstance() {
        static NoclipManager instance;
        return instance;
    }

    void hook(bool enable);
    void clear();

    void enableNoclip(Player* player);
    void disableNoclip(Player* player);
};

} // namespace coral_fans::functions
