#pragma once

#include "coral_fans/Config.h"
#include <vector>


namespace coral_fans::functions {

class ShortcutsManager {
private:
    std::vector<config::Shortcut::UseOn>   useons;
    std::vector<config::Shortcut::Use>     uses;
    std::vector<config::Shortcut::Destroy> destroys;
    std::vector<config::Shortcut::Command> commands;
#ifdef LL_PLAT_C
    std::vector<config::Shortcut::keyBoard> keyBoards;
#endif

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