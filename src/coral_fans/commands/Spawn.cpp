#include "coral_fans/commands/Commands.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/Level.h"

namespace coral_fans::commands {
void registerSpawnCommand(config::CommandConfigStruct& config) {
    if (config.enabled) {
        using ll::i18n_literals::operator""_tr;

        auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                        .getOrCreateCommand(config.command, "command.spawn.description"_tr(), config.permission);

        // spawn count global
        cmd.runtimeOverload().text("count").text("global").execute(
            [&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                const auto& spawner = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
                return output.success(
                    std::format("command.spawn.success.count.global"_tr(), spawner.mSpawnableMobTickCountPrevious)
                );
            }
        );
    }
}
} // namespace coral_fans::commands