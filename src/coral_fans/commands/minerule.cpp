#include "coral_fans/base/Mod.h"
#include "coral_fans/functions/minerule/drophook.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerMineruleCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    coral_fans::functions::DropHookManager::getInstance().bedrockDrop =
        coral_fans::mod().getConfigDb()->get("minerule.bedrockDrop") == "true";
    coral_fans::functions::DropHookManager::getInstance().mbDrop =
        coral_fans::mod().getConfigDb()->get("minerule.movingBlockDrop") == "true";

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
                    coral_fans::functions::DropHookManager::getInstance().bedrockDrop = true;
                    functions::DropHookManager::getInstance().dropHook();
                } else output.error("command.minerule.bedrockDrop.error"_tr());
            } else {
                if (coral_fans::mod().getConfigDb()->set("minerule.bedrockDrop", "false")) {
                    output.success("command.minerule.bedrockDrop.success.false"_tr());
                    coral_fans::functions::DropHookManager::getInstance().bedrockDrop = false;
                    functions::DropHookManager::getInstance().dropHook();
                } else output.error("command.minerule.bedrockDrop.error"_tr());
            }
        });

    mineruleCommand.runtimeOverload()
        .text("fuck_movingBlock_no_drop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (isopen) {
                if (coral_fans::mod().getConfigDb()->set("minerule.movingBlockDrop", "true")) {
                    output.success("command.minerule.movingBlockDrop.success.true"_tr());
                    coral_fans::functions::DropHookManager::getInstance().mbDrop = true;
                    functions::DropHookManager::getInstance().dropHook();
                } else output.error("command.minerule.movingBlockDrop.error"_tr());
            } else {
                if (coral_fans::mod().getConfigDb()->set("minerule.movingBlockDrop", "false")) {
                    output.success("command.minerule.movingBlockDrop.success.false"_tr());
                    coral_fans::functions::DropHookManager::getInstance().mbDrop = false;
                    functions::DropHookManager::getInstance().dropHook();
                } else output.error("command.minerule.movingBlockDrop.error"_tr());
            }
        });

    functions::DropHookManager::getInstance().dropHook();
}
} // namespace coral_fans::commands