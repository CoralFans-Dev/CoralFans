#pragma once

#include "coral_fans/Config.h"
#include <vector>


namespace coral_fans::functions {

class ShortcutsManager {
private:
    std::vector<coral_fans::config::Shortcut::UseOn>   useons;
    std::vector<coral_fans::config::Shortcut::Use>     uses;
    std::vector<coral_fans::config::Shortcut::Destroy> destroys;
    std::vector<coral_fans::config::Shortcut::Command> commands;

private:
    ShortcutsManager() = default;

public:
    [[nodiscard]] static ShortcutsManager& getInstance() {
        static ShortcutsManager instance;
        return instance;
    }

public:
    void registerShortcutsListener();
    void registerShortcutsCommand();
    void loadData();
    void clear();
};
} // namespace coral_fans::functions