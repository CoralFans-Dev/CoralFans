#include "coral_fans/base/Mod.h"
#include "coral_fans/functions/minerule/Drophook.h"
#include "coral_fans/functions/minerule/PortalSandFarm.h"
#include "coral_fans/functions/minerule/RemovePortalZombieCD.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerMineruleCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& mineruleCommand = ll::command::CommandRegistrar::getInstance()
                                .getOrCreateCommand("minerule", "command.minerule.description"_tr(), permission);

    mineruleCommand.runtimeOverload()
        .text("fuck_bedrock_no_drop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (isopen) {
                if (coral_fans::mod().getConfigDb()->set("minerule.bedrockDrop", "true")) {
                    output.success("command.minerule.bedrockDrop.success.true"_tr());
                    functions::bedrockDropHook(true);
                } else output.error("command.minerule.bedrockDrop.error"_tr());
            } else {
                if (coral_fans::mod().getConfigDb()->set("minerule.bedrockDrop", "false")) {
                    output.success("command.minerule.bedrockDrop.success.false"_tr());
                    functions::bedrockDropHook(false);
                } else output.error("command.minerule.bedrockDrop.error"_tr());
            }
        });
    functions::bedrockDropHook(mod().getConfigDb()->get("minerule.bedrockDrop") == "true");

    mineruleCommand.runtimeOverload()
        .text("fuck_movingBlock_no_drop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (isopen) {
                if (coral_fans::mod().getConfigDb()->set("minerule.movingBlockDrop", "true")) {
                    output.success("command.minerule.movingBlockDrop.success.true"_tr());
                    functions::mbDropHook(true);
                } else output.error("command.minerule.movingBlockDrop.error"_tr());
            } else {
                if (coral_fans::mod().getConfigDb()->set("minerule.movingBlockDrop", "false")) {
                    output.success("command.minerule.movingBlockDrop.success.false"_tr());
                    functions::mbDropHook(false);
                } else output.error("command.minerule.movingBlockDrop.error"_tr());
            }
        });

    functions::mbDropHook(mod().getConfigDb()->get("minerule.movingBlockDrop") == "true");

    mineruleCommand.runtimeOverload()
        .text("replicated_portal_sand_farm")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set(
                    "minerule.replicated_portal_sand_farm",
                    isopen ? "true" : "false"
                )) {
                output.success("command.minerule.replicated_portal_sand_farm.success"_tr(isopen ? "true" : "false"));
                functions::hook_portal_sand_farm(isopen);
            } else output.error("command.minerule.replicated_portal_sand_farm.error"_tr());
        });

    functions::hook_portal_sand_farm(mod().getConfigDb()->get("minerule.replicated_portal_sand_farm") == "true");

    mineruleCommand.runtimeOverload()
        .text("remove_portal_pigzombie_cd")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set(
                    "minerule.remove_portal_pigzombie_cd",
                    isopen ? "true" : "false"
                )) {
                output.success("command.minerule.remove_portal_pigzombie_cd.success"_tr(isopen ? "true" : "false"));
                functions::portal_spawn_hook(isopen);
            } else output.error("command.minerule.remove_portal_pigzombie_cd.error"_tr());
        });

    functions::portal_spawn_hook(mod().getConfigDb()->get("minerule.remove_portal_pigzombie_cd") == "true");
}
} // namespace coral_fans::commands