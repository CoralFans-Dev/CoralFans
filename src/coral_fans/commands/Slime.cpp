#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/player/Player.h"

namespace coral_fans::commands {

void registerSlimeCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& slimeCommand = ll::command::CommandRegistrar::getInstance()
                             .getOrCreateCommand("slime", "command.slime.description"_tr(), permission);

    // slime show <bool>
    slimeCommand.runtimeOverload()
        .text("show")
        .optional("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& slimeManager = coral_fans::mod().getSlimeManager();
            if (self["isopen"].has_value()) slimeManager.mShow = self["isopen"].get<ll::command::ParamKind::Bool>();
            else slimeManager.mShow = !slimeManager.mShow;
            if (!slimeManager.mShow) slimeManager.remove();
            output.success("command.slime.show.output"_tr(slimeManager.mShow ? "true" : "false"));
        });

    // slime check
    slimeCommand.overload().text("check").execute([](CommandOrigin const& origin, CommandOutput& output) {
        COMMAND_CHECK_PLAYER
        auto         pos  = utils::blockPosToChunkPos(player->getFeetBlockPos());
        auto         seed = (pos.x * 0x1f1f1f1fu) ^ (uint32_t)(pos.z);
        std::mt19937 mt(seed);
        output.success("{}", mt() % 10 == 0 ? "true" : "false");
    });
}

} // namespace coral_fans::commands