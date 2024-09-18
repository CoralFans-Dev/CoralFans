#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/player/Player.h"

namespace coral_fans::commands {

void registerSlimeCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& slimeCommand = ll::command::CommandRegistrar::getInstance()
                             .getOrCreateCommand("slime", "command.slime.description"_tr(), permission);

    struct SlimeIsOpenParam {
        bool isopen;
    };

    // slime show <bool>
    slimeCommand.overload<SlimeIsOpenParam>().text("show").required("isopen").execute(
        [](CommandOrigin const&, CommandOutput& output, SlimeIsOpenParam const& param) {
            if (!param.isopen) coral_fans::mod().getSlimeManager().remove();
            coral_fans::mod().getSlimeManager().setShow(param.isopen);
            output.success("command.slime.show.output"_tr(param.isopen ? "true" : "false"));
        }
    );

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