#include "Commands.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/MySchedule.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/platform/UUID.h"
#include "mc/profile/ProfilerLite.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandRegistry.h"
#include "mc/util/Timer.h"
#include "mc/world/Minecraft.h"
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
            const auto val = self["tickFreezeType"].get<ll::command::ParamKind::Enum>();
            // LevelEventPacket{LevelEvent::SimTimeStep, origin.getWorldPosition(), pause}.sendToClients();
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) {
                if (val.index) mc->setSimTimePause(true);
                else {
                    mc->setSimTimePause(false);
                    mc->setSimTimeScale(1.0f);
                }
            }
            output.success("command.tick.set.output"_tr(val.name));
        });

    // tick rate <float>
    tickCommand.runtimeOverload()
        .text("rate")
        .required("rate", ll::command::ParamKind::Float)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            float rate = self["rate"].get<ll::command::ParamKind::Float>();
            if (rate < 0) output.error("command.tick.rate.error"_tr());
            // LevelEventPacket{LevelEvent::SimTimeScale, {rate / 20}, rate > 0}.sendToClients();
            auto mc = ll::service::getMinecraft();

            mc->setSimTimePause(false);
            if (mc.has_value()) mc->setSimTimeScale(rate / 20.0f);
            output.success("command.tick.rate.success"_tr(rate));
        });

    // tick query [int]
    tickCommand.runtimeOverload()
        .text("query")
        .optional("times", ll::command::ParamKind::Int)
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
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
                if (uuid != "") {
                    auto player = ll::service::getLevel()->getPlayer(mce::UUID(uuid));
                    if (player)
                        TextPacket::createRawMessage(
                            "command.tick.query.output"_tr(
                                ProfilerLite::gProfilerLiteInstance().mDebugServerTickTime->count() / 1000000.0
                            )
                        )
                            .sendTo(*player);
                    else return false; // 玩家不在线了，停止任务
                } else
                    CoralFans::getInstance().getSelf().getLogger().info("command.tick.query.output"_tr(
                        ProfilerLite::gProfilerLiteInstance().mDebugServerTickTime->count() / 1000000.0
                    ));

                count++;
                return tick > count;
            });
        });

    // tick step <int>
    tickCommand.runtimeOverload()
        .text("step")
        .required("time", ll::command::ParamKind::Int)
        .execute([&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            int tick = self["time"].get<ll::command::ParamKind::Int>();
            if (!::Command::validRange(tick, 1, INT_MAX, output)) {
                return;
            }
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) mc->mSimTimer.mSteppingTick = (float)tick;
            output.success("command.tick.step.output"_tr(tick));
        });
}
} // namespace coral_fans::commands
