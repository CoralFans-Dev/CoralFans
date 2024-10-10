#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerHsaCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& hsaCommand = ll::command::CommandRegistrar::getInstance()
                           .getOrCreateCommand("hsa", "command.hsa.description"_tr(), permission);

    // hsa show <bool>
    hsaCommand.runtimeOverload()
        .text("show")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (!isopen) coral_fans::mod().getHsaManager().remove();
            coral_fans::mod().getHsaManager().setShow(isopen);
            output.success("command.hsa.show.output"_tr(isopen ? "true" : "false"));
        });
}
} // namespace coral_fans::commands