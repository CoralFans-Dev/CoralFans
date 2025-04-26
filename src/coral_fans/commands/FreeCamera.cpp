
#include "coral_fans/functions/freeCamera/FreeCamera.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/player/Player.h"


namespace coral_fans::commands {
void registerFreeCameraCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("freecamera", "command.freecamera.description"_tr(), permission);
    ll::service::getCommandRegistry()->registerAlias("freecamera", "fc");
    cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
        COMMAND_CHECK_PLAYER
        auto guid = player->getNetworkIdentifier().mGuid.g;
        if (!coral_fans::functions::FreeCameraManager::getInstance().FreeCamList.count(guid)) {
            coral_fans::functions::FreeCameraManager::EnableFreeCamera(player);
            return output.success("command.freecamera.enabled"_tr());
        } else {
            coral_fans::functions::FreeCameraManager::DisableFreeCamera(player);
            return output.success("command.freecamera.disabled"_tr());
        }
        return;
    });

    coral_fans::functions::FreeCameraManager::freecameraHook(true);
}
} // namespace coral_fans::commands