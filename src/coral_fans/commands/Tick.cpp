#include "Commands.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/MySchedule.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/tick/TickCommandManager.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/platform/UUID.h"
#include "mc/profile/ProfilerLite.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandRegistry.h"
#include "mc/util/Timer.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"


namespace coral_fans::commands {
void registerTickCommand(config::CommandConfigStruct& config) {
    if (!config.enabled) return;
    using ll::i18n_literals::operator""_tr;

    auto command = config.command;

    // reg cmd
    auto& tickCommand = ll::command::CommandRegistrar::getInstance(false)
                            .getOrCreateCommand(command, "command.tick.description"_tr(), config.permission);

    // tick freeze|reset
    ll::command::CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(
        "tickFreezeType",
        {
            {"reset",  0},
            {"freeze", 1}
    }
    );
    tickCommand.runtimeOverload()
        .required("tickFreezeType", ll::command::ParamKind::Enum, "tickFreezeType")
        .execute([&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            const auto val = self["tickFreezeType"].get<ll::command::ParamKind::Enum>();
            auto       mc  = ll::service::getMinecraft();
            if (mc.has_value()) [[likely]] {
                if (val.index) {
                    mc->setSimTimePause(true);
                    output.success("command.tick.set.freeze"_tr(val.name));
                } else {
                    mc->setSimTimePause(false);
                    mc->setSimTimeScale(1.0f);
                    functions::TickCommandManager::getInstance().applyReset();
                    output.success("command.tick.set.reset"_tr(val.name));
                }
            } else return output.error("command.tick.rate.error.generic"_tr());
        });

    // tick rate <float>
    tickCommand.runtimeOverload()
        .text("rate")
        .required("rate", ll::command::ParamKind::Float)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            float rate = self["rate"].get<ll::command::ParamKind::Float>();
            if (rate < 0) {
                output.error("command.tick.rate.error.outofrange"_tr(0.0f, rate));
                return;
            }
            auto mc = ll::service::getMinecraft();
            if (!mc.has_value()) [[unlikely]]
                return output.error("command.tick.rate.error.generic"_tr());

            mc->setSimTimePause(false);
            mc->setSimTimeScale(rate / 20.0f);
            functions::TickCommandManager::getInstance().applyRateChange(rate / 20.0f);
            output.success("command.tick.rate.success"_tr(rate));
        });

    // tick sync [bool]
    tickCommand.runtimeOverload()
        .text("sync")
        .optional("enable", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            COMMAND_CHECK_PLAYER
            auto& manager = functions::TickCommandManager::getInstance();
            bool  enable  = self["enable"].has_value() ? self["enable"].get<ll::command::ParamKind::Bool>()
                                                       : !manager.isSynced(player->getRealName());
            manager.setSync(*player, enable);
            output.success("command.tick.sync.output"_tr(enable ? "true" : "false"));
            utils::segmentAndSendToPlayer("§e" + "command.tick.sync.warning"_tr(), player);
        });

    // tick query [int]
    tickCommand.runtimeOverload()
        .text("query")
        .optional("times", ll::command::ParamKind::Int)
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            int tick = self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1;
            if (!::Command::validRange(tick, 0, INT_MAX, output)) {
                return;
            }
            auto player = tryGetPlayer(origin);
            if (!player.has_value()) {
                output.error("command.tick.query.error"_tr());
                return;
            }
            auto uuid = player.value() ? player.value()->getUuid().asString() : "";
            my_schedule::MySchedule::getSchedule().add([uuid, tick](int&, int& count) {
                auto message = "command.tick.query.mspt"_tr(
                    ProfilerLite::gProfilerLiteInstance().mDebugServerTickTime->count() / 1000000.0
                );
                if (auto mc = ll::service::getMinecraft(); mc && mc->mSimTimer.mSteppingTick > 1e-4) {
                    message += "command.tick.query.targetmspt"_tr(1000.0f / mc->mSimTimer.mTimeScale * 20);
                }
                if (uuid != "") {
                    auto player = ll::service::getLevel()->getPlayer(mce::UUID(uuid));
                    if (player) utils::segmentAndSendToPlayer(message, player);
                    else return false; // 玩家不在线了，停止任务
                } else CoralFans::getInstance().getSelf().getLogger().info(message);

                count++;
                return tick > count;
            });
        });

    // tick step <int>
    tickCommand.runtimeOverload()
        .text("step")
        .required("time", ll::command::ParamKind::Int)
        .execute([&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            int tick = self["time"].get<ll::command::ParamKind::Int>();
            if (!::Command::validRange(tick, 1, INT_MAX, output)) {
                return;
            }
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) [[likely]] {
                mc->mSimTimer.mSteppingTick = (float)tick;
                functions::TickCommandManager::getInstance().applyReset();
                output.success("command.tick.step.output"_tr(tick));
            } else return output.error("command.tick.rate.error.generic"_tr());
        });

    functions::TickCommandManager::getInstance().hook(true);
}
} // namespace coral_fans::commands
