#include "coral_fans/functions/prof/Prof.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/ParamTraits.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>


namespace coral_fans::commands {

void registerProfCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& profCommand = ll::command::CommandRegistrar::getInstance()
                            .getOrCreateCommand("prof", "command.prof.description"_tr(), permission);

    ll::command::CommandRegistrar::getInstance().tryRegisterRuntimeEnum("profType", functions::Profiler::TypeVec);
    profCommand.runtimeOverload()
        .optional("type", ll::command::ParamKind::Enum, "profType")
        .optional("numberOfTick", ll::command::ParamKind::Int)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            uint64 type         = functions::Profiler::Type::normal;
            int    numberOfTick = 100;
            if (self["type"].has_value()) type = self["type"].get<ll::command::ParamKind::Enum>().index;
            if (self["numberOfTick"].has_value())
                numberOfTick = self["numberOfTick"].get<ll::command::ParamKind::Int>();
            if (numberOfTick <= 0 || numberOfTick > 1200) return output.error("command.prof.error.outofrange"_tr());
            if (coral_fans::mod().getProfiler().profiling) return output.error("command.prof.error.running"_tr());
            coral_fans::mod().getProfiler().start(numberOfTick, type);
            output.success("command.prof.success"_tr());
        });
}

} // namespace coral_fans::commands