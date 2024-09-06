#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/entity/utilities/ActorType.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"


#define CHECK_PLAYER                                                                                                   \
    auto* entity = origin.getEntity();                                                                                 \
    if (entity == nullptr || !entity->isType(ActorType::Player)) {                                                     \
        output.error("Only players can run this command");                                                             \
        return;                                                                                                        \
    }                                                                                                                  \
    auto* player = static_cast<Player*>(entity);

namespace coral_fans::commands {
void registerSelfCommand() {
    // reg cmd
    auto& selfCommand = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "self",
        "Controls functions of CoralFans Mod.",
        CommandPermissionLevel::Any
    );

    struct SelfIsOpenParam {
        bool isopen;
    };

    // self noclip <bool>
    selfCommand.overload<SelfIsOpenParam>().text("noclip").required("isopen").execute(
        [](CommandOrigin const& origin, CommandOutput& output, SelfIsOpenParam const& param) {
            CHECK_PLAYER
            const auto global = coral_fans::mod().getConfigDb()->get("functions.global.noclip") == "true";
            const bool rst    = coral_fans::mod().getConfigDb()->set(
                std::format("functions.players.{}.noclip", player->getUuid().asString()),
                (param.isopen & global) ? "true" : "false"
            );
            if (rst) output.success("set noclip \"{}\"", (param.isopen & global) ? "true" : "false");
            else output.error("failed to set noclip");
        }
    );
}
} // namespace coral_fans::commands