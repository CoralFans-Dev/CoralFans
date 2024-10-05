#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"

#include "coral_fans/functions/HopperCounter.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/Optional.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/material/Material.h"

namespace coral_fans::commands {

void registerCounterCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& counterCommand = ll::command::CommandRegistrar::getInstance()
                               .getOrCreateCommand("counter", "command.counter.description"_tr(), permission);

    struct CounterChannelParam {
        ll::command::Optional<int> channel;
    };

    // count print <int>
    counterCommand.overload<CounterChannelParam>().text("print").optional("channel").execute(
        [](CommandOrigin const& origin, CommandOutput& output, CounterChannelParam const& param) {
            if (param.channel.has_value()) {
                output.success(coral_fans::mod().getHopperCounterManager().getChannel(param.channel).info());
            } else {
                COMMAND_CHECK_PLAYER
                auto hitrst = player->traceRay(5.25f, false, true, [](BlockSource const&, Block const& block, bool) {
                    if (block.getMaterial().isLiquid()) return false;
                    return true;
                });
                if (!hitrst) return output.error("command.counter.print.error"_tr());
                auto& blockSource = player->getDimensionBlockSource();
                int   ch          = functions::HopperCounterManager::getViewChannel(blockSource, hitrst);
                if (ch == -1) return output.error("command.counter.print.error"_tr());
                output.success(coral_fans::mod().getHopperCounterManager().getChannel(ch).info());
            }
        }
    );

    // count reset <int>
    counterCommand.overload<CounterChannelParam>().text("reset").optional("channel").execute(
        [](CommandOrigin const& origin, CommandOutput& output, CounterChannelParam const& param) {
            if (param.channel.has_value()) {
                coral_fans::mod().getHopperCounterManager().getChannel(param.channel).reset();
                output.success("command.counter.reset.success"_tr(param.channel.value()));
            } else {
                COMMAND_CHECK_PLAYER
                auto hitrst = player->traceRay(5.25f, false, true, [](BlockSource const&, Block const& block, bool) {
                    if (block.getMaterial().isLiquid()) return false;
                    return true;
                });
                if (!hitrst) return output.error("command.counter.print.error"_tr());
                auto& blockSource = player->getDimensionBlockSource();
                int   ch          = functions::HopperCounterManager::getViewChannel(blockSource, hitrst);
                if (ch == -1) return output.error("command.counter.reset.error"_tr(ch));
                coral_fans::mod().getHopperCounterManager().getChannel(ch).reset();
                output.success("command.counter.reset.success"_tr(ch));
            }
        }
    );
}

} // namespace coral_fans::commands