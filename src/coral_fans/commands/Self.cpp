#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"

namespace coral_fans::commands {
void registerSelfCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& selfCommand = ll::command::CommandRegistrar::getInstance()
                            .getOrCreateCommand("self", "command.self.description"_tr(), permission);

    // self noclip <bool>
    selfCommand.runtimeOverload()
        .text("noclip")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.noclip") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.noclip", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.noclip.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.noclip.error"_tr());
        });

    // self autotool <bool>
    selfCommand.runtimeOverload()
        .text("autotool")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.autotool") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autotool", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.autotool.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.autotool.error"_tr());
        });

    // self autotool mindamage <int>
    selfCommand.runtimeOverload()
        .text("autotool")
        .text("mindamage")
        .required("mindamage", ll::command::ParamKind::Int)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            auto minDamageString = std::to_string(self["mindamage"].get<ll::command::ParamKind::Int>());
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autotool.mindamage", player->getUuid().asString()),
                    minDamageString
                ))
                output.success("command.self.autotool.mindamage.success"_tr(minDamageString));
            else output.error("command.self.autotool.mindamage.error"_tr());
        });

    // self containerreader <bool>
    selfCommand.runtimeOverload()
        .text("containerreader")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.containerreader") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.containerreader", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.containerreader.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.containerreader.error"_tr());
        });

    // self autototem <bool>
    selfCommand.runtimeOverload()
        .text("autototem")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.autototem") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autototem", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.autototem.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.autototem.error"_tr());
        });

    // self autoitem <bool>
    selfCommand.runtimeOverload()
        .text("autoitem")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.autoitem") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autoitem", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.autoitem.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.autoitem.error"_tr());
        });

    // self fastdrop <bool>
    selfCommand.runtimeOverload()
        .text("fastdrop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.fastdrop") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.fastdrop", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.fastdrop.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.fastdrop.error"_tr());
        });

    // self nopickup <bool>
    selfCommand.runtimeOverload()
        .text("nopickup")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            bool       isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.nopickup") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.nopickup", player->getUuid().asString()),
                    (isopen & global) ? "true" : "false"
                ))
                output.success("command.self.nopickup.success"_tr((isopen & global) ? "true" : "false"));
            else output.error("command.self.nopickup.error"_tr());
        });
}
} // namespace coral_fans::commands