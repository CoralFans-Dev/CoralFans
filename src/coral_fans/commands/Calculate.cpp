#include "coral_fans/functions/calculate/Calculate.h"
#include "coral_fans/Config.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"


namespace coral_fans::commands {

void registerCalculateCommand(config::CommandConfigStruct& config) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& calculateCommand = ll::command::CommandRegistrar::getInstance(false).getOrCreateCommand(
        config.command,
        "command.calculate.description"_tr(),
        config.permission
    );
    calculateCommand.runtimeOverload().text("pt").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            functions::calculatePt(player);
        }
    );

    calculateCommand.runtimeOverload().text("pt2").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            functions::calculatePt2(player);
        }
    );
}
} // namespace coral_fans::commands