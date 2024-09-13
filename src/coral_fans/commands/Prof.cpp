#include "coral_fans/functions/Prof.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/Optional.h"
#include "ll/api/i18n/I18n.h"
#include "magic_enum.hpp"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>

namespace coral_fans::commands {

void registerProfCommand(std::string permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& profCommand = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "prof",
        "command.prof.description"_tr(),
        magic_enum::enum_cast<CommandPermissionLevel>(permission).value_or(CommandPermissionLevel::Any)
    );

    enum class ProfType : int { normal, entity, chunk, pt };
    struct ProfParam {
        ll::command::Optional<ProfType> type;
        ll::command::Optional<int>      numberOfTick;
    };
    profCommand.overload<ProfParam>()
        .optional("type")
        .optional("numberOfTick")
        .execute([](CommandOrigin const&, CommandOutput& output, ProfParam const& param) {
            auto type         = param.type.value_or(ProfType::normal);
            int  numberOfTick = param.numberOfTick.value_or(100);
            if (numberOfTick <= 0 || numberOfTick > 1200) return output.error("command.prof.error.outofrange"_tr());
            if (coral_fans::mod().getProfiler().profiling) return output.error("command.prof.error.running"_tr());
            coral_fans::mod().getProfiler().start(numberOfTick, static_cast<functions::Profiler::Type>(type));
            output.success("command.prof.success"_tr());
        });
}

} // namespace coral_fans::commands