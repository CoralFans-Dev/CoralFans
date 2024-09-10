#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerHsaCommand(std::string permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& hsaCommand = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "hsa",
        "command.hsa.description"_tr(),
        magic_enum::enum_cast<CommandPermissionLevel>(permission).value_or(CommandPermissionLevel::GameDirectors)
    );

    struct HsaIsOpenParam {
        bool isopen;
    };

    // hsa show <bool>
    hsaCommand.overload<HsaIsOpenParam>().text("show").required("isopen").execute(
        [](CommandOrigin const&, CommandOutput& output, HsaIsOpenParam const& param) {
            if (!param.isopen) coral_fans::mod().getHsaManager().remove();
            if (coral_fans::mod().getConfigDb()->set("functions.data.hsa.show", param.isopen ? "true" : "false"))
                output.success("command.hsa.show.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.hsa.show.error"_tr());
        }
    );
}
} // namespace coral_fans::commands