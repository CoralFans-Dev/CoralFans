#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"

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
#include <unordered_map>

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
                auto  hitrst = player->traceRay(5.25f, false, true, [](BlockSource const&, Block const& block, bool) {
                    if (block.getMaterial().isLiquid()) return false;
                    return true;
                });
                auto& blockSource = player->getDimension().getBlockSourceFromMainChunkSource();
                const auto&                                          dest = blockSource.getBlock(hitrst.mBlockPos);
                std::unordered_map<std::string, int>::const_iterator it;
                if (utils::removeMinecraftPrefix(dest.getTypeName()) == "hopper") {
                    const auto& block =
                        blockSource.getBlock(hitrst.mBlockPos + utils::facingToBlockPos(dest.getVariant()));
                    it = functions::HopperCounterManager::HOPPER_COUNTER_MAP.find(
                        utils::removeMinecraftPrefix(block.getTypeName())
                    );
                } else
                    it = functions::HopperCounterManager::HOPPER_COUNTER_MAP.find(
                        utils::removeMinecraftPrefix(dest.getTypeName())
                    );
                if (it == functions::HopperCounterManager::HOPPER_COUNTER_MAP.end())
                    return output.error("command.counter.print.error"_tr());
                output.success(coral_fans::mod().getHopperCounterManager().getChannel(it->second).info());
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
                auto  hitrst = player->traceRay(5.25f, false, true, [](BlockSource const&, Block const& block, bool) {
                    if (block.getMaterial().isLiquid()) return false;
                    return true;
                });
                auto& blockSource = player->getDimension().getBlockSourceFromMainChunkSource();
                const auto&                                          dest = blockSource.getBlock(hitrst.mBlockPos);
                std::unordered_map<std::string, int>::const_iterator it;
                if (utils::removeMinecraftPrefix(dest.getTypeName()) == "hopper") {
                    const auto& block =
                        blockSource.getBlock(hitrst.mBlockPos + utils::facingToBlockPos(dest.getVariant()));
                    it = functions::HopperCounterManager::HOPPER_COUNTER_MAP.find(
                        utils::removeMinecraftPrefix(block.getTypeName())
                    );
                } else
                    it = functions::HopperCounterManager::HOPPER_COUNTER_MAP.find(
                        utils::removeMinecraftPrefix(dest.getTypeName())
                    );
                if (it == functions::HopperCounterManager::HOPPER_COUNTER_MAP.end())
                    return output.error("command.counter.reset.error"_tr(it->second));
                coral_fans::mod().getHopperCounterManager().getChannel(it->second).reset();
                output.success("command.counter.reset.success"_tr(it->second));
            }
        }
    );
}

} // namespace coral_fans::commands