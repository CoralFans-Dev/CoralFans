#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
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
    cfhudCommand.runtimeOverload()
        .text("show")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            if (coral_fans::mod().getConfigDb()->set(
                    "functions.players." + player->getUuid().asString() + ".cfhud.show",
                    self["isopen"].get<ll::command::ParamKind::Bool>() ? "true" : "false"
                ))
                output.success("command.cfhud.success"_tr());
            else output.error("command.cfhud.error.seterror"_tr());
        });

    // cfhud <add|remove> <mspt|base|redstone|village>
    ll::command::CommandRegistrar::getInstance().tryRegisterEnum(
        "cfhudActionType",
        {
            {"add", 0},
            {"remove", 1}
        },
        Bedrock::type_id<CommandRegistry, std::pair<std::string,uint64>>(),
        &CommandRegistry::parse<std::pair<std::string,uint64>>
    );
    ll::command::CommandRegistrar::getInstance()
        .tryRegisterEnum("cfhudType", functions::HudHelper::HudTypeVec, Bedrock::type_id<CommandRegistry, std::pair<std::string, uint64>>(), &CommandRegistry::parse<std::pair<std::string, uint64>>);
    cfhudCommand.runtimeOverload()
        .required("action", ll::command::ParamKind::Enum, "cfhudActionType")
        .required("hud", ll::command::ParamKind::Enum, "cfhudType")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            auto&         mod = coral_fans::mod();
            unsigned long hud;
            try {
                hud = std::stoul(mod.getConfigDb()
                                     ->get("functions.players." + player->getUuid().asString() + ".cfhud.hud")
                                     .value_or("0"));
            } catch (...) {
                return output.error("command.cfhud.error.geterror"_tr());
            }
            if (self["action"].get<ll::command::ParamKind::Enum>().index == 0)
                hud |= (1 << self["hud"].get<ll::command::ParamKind::Enum>().index);
            else hud &= ~(1 << self["hud"].get<ll::command::ParamKind::Enum>().index);
            if (mod.getConfigDb()
                    ->set("functions.players." + player->getUuid().asString() + ".cfhud.hud", std::to_string(hud)))
                output.success("command.cfhud.success"_tr());
            else output.error("command.cfhud.error.seterror"_tr());
        });
}

} // namespace coral_fans::commands