#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/functions/noclip/NoclipManager.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/GameType.h"

namespace coral_fans::commands {
void registerNoclipCommand(config::CommandConfigStruct& config) {
    if (config.enabled) {
        using ll::i18n_literals::operator""_tr;

        auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                        .getOrCreateCommand(config.command, "command.noclip.description"_tr(), config.permission);
        cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
            using ll::i18n_literals::operator""_tr;
            COMMAND_CHECK_PLAYER
            if (player->getPlayerGameType() != GameType::Creative) return;
            auto& abilities = player->getAbilities();
            bool  enable    = !abilities.getAbility(AbilitiesIndex::NoClip).mValue->mBoolVal;
            CoralFans::getInstance().getConfigDb()->set(
                std::format("noclip.players.{}", player->getUuid().asString()),
                enable ? "T" : "F"
            );
            if (enable) {
                functions::NoclipManager::getInstance().enableNoclip(player);
                output.success("command.noclip.enabled"_tr());
            } else {
                functions::NoclipManager::getInstance().disableNoclip(player);
                output.success("command.noclip.disabled"_tr());
            }
        });
    }

    functions::NoclipManager::getInstance().hook(config.enabled);
}
} // namespace coral_fans::commands
