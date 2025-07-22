
#include "coral_fans/functions/noclip/Noclip.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/MySchedule.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/GameType.h"


namespace coral_fans::commands {
void registerNoclipCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("noclip", "command.noclip.description"_tr(), permission);
    cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
        COMMAND_CHECK_PLAYER
        if (player->getPlayerGameType() != GameType::Creative) return;
        auto& abilities = player->getAbilities();
        bool  enable    = !abilities.getAbility(AbilitiesIndex::NoClip).mValue->mBoolVal;
        coral_fans::mod().getConfigDb()->set(
            std::format("noclip.players.{}", player->getUuid().asString()),
            enable ? "T" : "F"
        );
        if (enable) {
            player->setAbility(::AbilitiesIndex::Flying, true);
            my_schedule::MySchedule::getSchedule().add(3, [player, output]() {
                if (player) {
                    player->setAbility(::AbilitiesIndex::NoClip, true);
                }
            });
            output.success("command.noclip.enabled"_tr());
        } else {
            player->setAbility(::AbilitiesIndex::NoClip, false);
            player->setAbility(::AbilitiesIndex::Flying, true);
            output.success("command.noclip.disabled"_tr());
        }
    });

    coral_fans::functions::noclipHook(true);
}
} // namespace coral_fans::commands