#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
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

    struct HsaIsOpenParam {
        bool isopen;
    };

    // hsa show <bool>
    hsaCommand.overload<HsaIsOpenParam>().text("show").required("isopen").execute(
        [](CommandOrigin const&, CommandOutput& output, HsaIsOpenParam const& param) {
            if (!param.isopen) coral_fans::mod().getHsaManager().remove();
            coral_fans::mod().getHsaManager().setShow(param.isopen);
            output.success("command.hsa.show.output"_tr(param.isopen ? "true" : "false"));
        }
    );
}
} // namespace coral_fans::commands