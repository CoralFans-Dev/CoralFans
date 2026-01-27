#include "coral_fans/base/Mod.h"
#include "coral_fans/functions/minerule/MineruleManager.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerMineruleCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& mineruleCommand = ll::command::CommandRegistrar::getInstance(false)
                                .getOrCreateCommand("minerule", "command.minerule.description"_tr(), permission);

    auto& mineruleManager = coral_fans::functions::MineruleManager::getInstance();

    mineruleCommand.runtimeOverload()
        .text("fuck_bedrock_no_drop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute(
            [&mineruleManager](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (isopen) {
                    if (coral_fans::mod().getConfigDb()->set("minerule.bedrockDrop", "true")) {
                        output.success("command.minerule.bedrockDrop.success.true"_tr());
                        mineruleManager.bedrockDropHook(true);
                    } else output.error("command.minerule.bedrockDrop.error"_tr());
                } else {
                    if (coral_fans::mod().getConfigDb()->set("minerule.bedrockDrop", "false")) {
                        output.success("command.minerule.bedrockDrop.success.false"_tr());
                        mineruleManager.bedrockDropHook(false);
                    } else output.error("command.minerule.bedrockDrop.error"_tr());
                }
            }
        );
    mineruleManager.bedrockDropHook(coral_fans::mod().getConfigDb()->get("minerule.bedrockDrop") == "true");

    mineruleCommand.runtimeOverload()
        .text("fuck_movingBlock_no_drop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute(
            [&mineruleManager](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (isopen) {
                    if (coral_fans::mod().getConfigDb()->set("minerule.movingBlockDrop", "true")) {
                        output.success("command.minerule.movingBlockDrop.success.true"_tr());
                        mineruleManager.mbDropHook(true);
                    } else output.error("command.minerule.movingBlockDrop.error"_tr());
                } else {
                    if (coral_fans::mod().getConfigDb()->set("minerule.movingBlockDrop", "false")) {
                        output.success("command.minerule.movingBlockDrop.success.false"_tr());
                        mineruleManager.mbDropHook(false);
                    } else output.error("command.minerule.movingBlockDrop.error"_tr());
                }
            }
        );
    mineruleManager.mbDropHook(coral_fans::mod().getConfigDb()->get("minerule.movingBlockDrop") == "true");

    mineruleCommand.runtimeOverload()
        .text("restore_portal_sand_farm")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([&mineruleManager](
                     CommandOrigin const&,
                     CommandOutput&                     output,
                     ll::command::RuntimeCommand const& self
                 ) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("minerule.restore_portal_sand_farm", isopen ? "true" : "false")) {
                output.success("command.minerule.restore_portal_sand_farm.success"_tr(isopen ? "true" : "false"));
                mineruleManager.portalSandFarmHook(isopen);
            } else output.error("command.minerule.restore_portal_sand_farm.error"_tr());
        });

    mineruleManager.portalSandFarmHook(mod().getConfigDb()->get("minerule.restore_portal_sand_farm") == "true");

    mineruleCommand.runtimeOverload()
        .text("remove_portal_pigzombie_cd")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute(
            [&mineruleManager](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (coral_fans::mod().getConfigDb()->set(
                        "minerule.remove_portal_pigzombie_cd",
                        isopen ? "true" : "false"
                    )) {
                    output.success("command.minerule.remove_portal_pigzombie_cd.success"_tr(isopen ? "true" : "false"));
                    mineruleManager.portalSpawnHook(isopen);
                } else output.error("command.minerule.remove_portal_pigzombie_cd.error"_tr());
            }
        );

    mineruleManager.portalSpawnHook(mod().getConfigDb()->get("minerule.remove_portal_pigzombie_cd") == "true");

    mineruleCommand.runtimeOverload()
        .text("restore_ancillary_broken")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([&mineruleManager](
                     CommandOrigin const&,
                     CommandOutput&                     output,
                     ll::command::RuntimeCommand const& self
                 ) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("minerule.restore_ancillary_broken", isopen ? "true" : "false")) {
                output.success("command.minerule.restore_ancillary_broken.success"_tr(isopen ? "true" : "false"));
                mineruleManager.restoreAncillaryBrokenHook(isopen);
            } else output.error("command.minerule.restore_ancillary_broken.error"_tr());
        });

    mineruleManager.restoreAncillaryBrokenHook(mod().getConfigDb()->get("minerule.restore_ancillary_broken") == "true");
}
} // namespace coral_fans::commands