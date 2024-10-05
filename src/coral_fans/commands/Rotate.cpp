#include "coral_fans/functions/Rotate.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"

namespace coral_fans::commands {

void registerRotateCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& rotateCommand = ll::command::CommandRegistrar::getInstance()
                              .getOrCreateCommand("rotate", "command.rotate.description"_tr(), permission);

    rotateCommand.overload().execute([](CommandOrigin const& origin, CommandOutput& output) {
        COMMAND_CHECK_PLAYER
        auto hitrst = player->traceRay(5.25f, false, true);
        if (!hitrst) return;
        functions::rotateBlock(player, hitrst.mBlockPos);
    });
}

} // namespace coral_fans::commands