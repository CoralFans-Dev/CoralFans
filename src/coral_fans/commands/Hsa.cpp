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
        .optional("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& hsaManager = coral_fans::mod().getHsaManager();
            if (self["isopen"].has_value()) hsaManager.mShow = self["isopen"].get<ll::command::ParamKind::Bool>();
            else hsaManager.mShow = !hsaManager.mShow;
            if (!hsaManager.mShow) hsaManager.remove();
            output.success("command.hsa.show.output"_tr(hsaManager.mShow ? "true" : "false"));
        });
}
} // namespace coral_fans::commands