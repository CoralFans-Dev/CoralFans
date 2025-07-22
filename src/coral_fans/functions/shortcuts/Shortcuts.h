#pragma once

#include "coral_fans/Config.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandRegistry.h"
#include <vector>


namespace coral_fans::functions {

class ShortcutsManager {
private:
    std::vector<coral_fans::config::Shortcut::UseOn>   useons;
    std::vector<coral_fans::config::Shortcut::Use>     uses;
    std::vector<coral_fans::config::Shortcut::Destroy> destroys;
    std::vector<coral_fans::config::Shortcut::Command> commands;

private:
    ll::event::ListenerPtr playerJoinEventListener;

private:
    ShortcutsManager() {
        auto const& commandregistry = ll::service::getCommandRegistry();

        // useons
        for (auto& useon : coral_fans::mod().getConfig().useons) {
            if (!useon.enable || useon.item == "") continue;
            for (auto action : useon.actions) {
                if (!commandregistry->findCommand(action))
                    goto next; // 如果action中有一条未注册，则不会加入到shortcuts中
            }
            useons.push_back(useon);
        next:;
        }

        // uses
        for (auto& use : coral_fans::mod().getConfig().uses) {
            if (!use.enable || use.item == "") continue;
            for (auto action : use.actions) {
                if (!commandregistry->findCommand(action))
                    goto next2; // 如果action中有一条未注册，则不会加入到shortcuts中
            }
            uses.push_back(use);
        next2:;
        }

        // destroys
        for (auto& destroy : coral_fans::mod().getConfig().destroys) {
            if (!destroy.enable || destroy.item == "") continue;
            for (auto action : destroy.actions) {
                if (!commandregistry->findCommand(action))
                    goto next3; // 如果action中有一条未注册，则不会加入到shortcuts中
            }
            destroys.push_back(destroy);
        next3:;
        }

        // commands
        for (auto& command : coral_fans::mod().getConfig().commands) {
            if (!command.enable || command.command == "") continue;
            for (auto action : command.actions) {
                if (!commandregistry->findCommand(action))
                    goto next4; // 如果action中有一条未注册，则不会加入到shortcuts中
            }
            commands.push_back(command);
        next4:;
        }
    }

public:
    static ShortcutsManager& getInstance() {
        static ShortcutsManager instance;
        return instance;
    }

public:
    void waitToRegisterShortcuts();
    void registerShortcutsListener();
    void registerShortcutsCommand();
};
} // namespace coral_fans::functions