#include "coral_fans/functions/calculate/Calculate.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {

void registerCalculateCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& calculateCommand = ll::command::CommandRegistrar::getInstance(false)
                                 .getOrCreateCommand("calculate", "command.calculate.description"_tr(), permission);
    calculateCommand.runtimeOverload().text("pt").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            functions::calculatePt(player, output);
        }
    );

    calculateCommand.runtimeOverload().text("pt2").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            functions::calculatePt2(player, output);
        }
    );
}
} // namespace coral_fans::commands