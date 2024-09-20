#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
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

    struct SelfIsOpenParam {
        bool isopen;
    };

    // self noclip <bool>
    selfCommand.overload<SelfIsOpenParam>().text("noclip").required("isopen").execute(
        [](CommandOrigin const& origin, CommandOutput& output, SelfIsOpenParam const& param) {
            COMMAND_CHECK_PLAYER
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.noclip") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.noclip", player->getUuid().asString()),
                    (param.isopen & global) ? "true" : "false"
                ))
                output.success("command.self.noclip.success"_tr((param.isopen & global) ? "true" : "false"));
            else output.error("command.self.noclip.error"_tr());
        }
    );

    // self autotool <bool>
    selfCommand.overload<SelfIsOpenParam>()
        .text("autotool")
        .required("isopen")
        .execute([](CommandOrigin const& origin, CommandOutput& output, SelfIsOpenParam const& param) {
            COMMAND_CHECK_PLAYER
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.autotool") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autotool", player->getUuid().asString()),
                    (param.isopen & global) ? "true" : "false"
                ))
                output.success("command.self.autotool.success"_tr((param.isopen & global) ? "true" : "false"));
            else output.error("command.self.autotool.error"_tr());
        });

    // self autotool mindamage <int>
    struct SelfAutoToolMinDamageParam {
        int mindamage;
    };
    selfCommand.overload<SelfAutoToolMinDamageParam>()
        .text("autotool")
        .text("mindamage")
        .required("mindamage")
        .execute([](CommandOrigin const& origin, CommandOutput& output, SelfAutoToolMinDamageParam const& param) {
            COMMAND_CHECK_PLAYER
            auto minDamageString = std::to_string(param.mindamage);
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autotool.mindamage", player->getUuid().asString()),
                    minDamageString
                ))
                output.success("command.self.autotool.mindamage.success"_tr(minDamageString));
            else output.error("command.self.autotool.mindamage.error"_tr());
        });

    // self containerreader <bool>
    selfCommand.overload<SelfIsOpenParam>()
        .text("containerreader")
        .required("isopen")
        .execute([](CommandOrigin const& origin, CommandOutput& output, SelfIsOpenParam const& param) {
            COMMAND_CHECK_PLAYER
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.containerreader") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.containerreader", player->getUuid().asString()),
                    (param.isopen & global) ? "true" : "false"
                ))
                output.success("command.self.containerreader.success"_tr((param.isopen & global) ? "true" : "false"));
            else output.error("command.self.containerreader.error"_tr());
        });

    // self autototem <bool>
    selfCommand.overload<SelfIsOpenParam>()
        .text("autototem")
        .required("isopen")
        .execute([](CommandOrigin const& origin, CommandOutput& output, SelfIsOpenParam const& param) {
            COMMAND_CHECK_PLAYER
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.autototem") == "true";
            if (coral_fans::mod().getConfigDb()->set(
                    std::format("functions.players.{}.autototem", player->getUuid().asString()),
                    (param.isopen & global) ? "true" : "false"
                ))
                output.success("command.self.autototem.success"_tr((param.isopen & global) ? "true" : "false"));
            else output.error("command.self.autototem.error"_tr());
        });
}
} // namespace coral_fans::commands