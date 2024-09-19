#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/functions/Hud.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"
#include <string>

namespace coral_fans::commands {

void registerCfhudCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;
    // reg cmd
    auto& cfhudCommand = ll::command::CommandRegistrar::getInstance()
                             .getOrCreateCommand("cfhud", "command.cfhud.description"_tr(), permission);

    // cfhud show <bool>
    struct CfhudShowParam {
        bool isopen;
    };
    cfhudCommand.overload<CfhudShowParam>().text("show").required("isopen").execute(
        [](CommandOrigin const& origin, CommandOutput& output, CfhudShowParam const& param) {
            COMMAND_CHECK_PLAYER
            if (coral_fans::mod().getConfigDb()->set(
                    "functions.players." + player->getUuid().asString() + ".cfhud.show",
                    param.isopen ? "true" : "false"
                ))
                output.success("command.cfhud.success"_tr());
            else output.error("command.cfhud.error.seterror"_tr());
        }
    );

    // cfhud <add|remove> <mspt|base|redstone|village>
    enum class CfhudActionType : int { add, remove };
    struct CfhudSetParam {
        CfhudActionType               action;
        functions::HudHelper::HudType hud;
    };
    cfhudCommand.overload<CfhudSetParam>().required("action").required("hud").execute([](CommandOrigin const& origin,
                                                                                         CommandOutput&       output,
                                                                                         CfhudSetParam const& param) {
        COMMAND_CHECK_PLAYER
        auto&         mod = coral_fans::mod();
        unsigned long hud;
        try {
            hud = std::stoul(
                mod.getConfigDb()->get("functions.players." + player->getUuid().asString() + ".cfhud.hud").value_or("0")
            );
        } catch (...) {
            return output.error("command.cfhud.error.geterror"_tr());
        }
        if (param.action == CfhudActionType::add) hud |= (1 << param.hud);
        else hud &= ~(1 << param.hud);
        if (mod.getConfigDb()
                ->set("functions.players." + player->getUuid().asString() + ".cfhud.hud", std::to_string(hud)))
            output.success("command.cfhud.success"_tr());
        else output.error("command.cfhud.error.seterror"_tr());
    });
}

} // namespace coral_fans::commands