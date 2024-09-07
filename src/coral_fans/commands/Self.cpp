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
void registerSelfCommand() {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& selfCommand = ll::command::CommandRegistrar::getInstance()
                            .getOrCreateCommand("self", "command.self.description"_tr(), CommandPermissionLevel::Any);

    struct SelfIsOpenParam {
        bool isopen;
    };

    // self noclip <bool>
    selfCommand.overload<SelfIsOpenParam>().text("noclip").required("isopen").execute(
        [](CommandOrigin const& origin, CommandOutput& output, SelfIsOpenParam const& param) {
            COMMAND_CHECK_PLAYER
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.noclip") == "true";
            const bool rst    = coral_fans::mod().getConfigDb()->set(
                std::format("functions.players.{}.noclip", player->getUuid().asString()),
                (param.isopen & global) ? "true" : "false"
            );
            if (rst) output.success("command.self.noclip.success"_tr((param.isopen & global) ? "true" : "false"));
            else output.error("command.self.noclip.error"_tr());
        }
    );
}
} // namespace coral_fans::commands