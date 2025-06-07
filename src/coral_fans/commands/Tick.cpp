#include "coral_fans/base/MySchedule.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandRegistry.h"
#include "mc/util/Timer.h"
#include "mc/world/Minecraft.h"


namespace coral_fans::commands {
void registerTickCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& tickCommand = ll::command::CommandRegistrar::getInstance()
                            .getOrCreateCommand("tick", "command.tick.description"_tr(), permission);

    // tick freeze|reset
    ll::command::CommandRegistrar::getInstance().tryRegisterRuntimeEnum(
        "tickFreezeType",
        {
            {"reset",  0},
            {"freeze", 1}
    }
    );
    tickCommand.runtimeOverload()
        .required("tickFreezeType", ll::command::ParamKind::Enum, "tickFreezeType")
        .execute([&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool       pause = false;
            const auto val   = self["tickFreezeType"].get<ll::command::ParamKind::Enum>();
            switch (val.index) {
            case 1:
                pause = true;
                break;
            case 0:
                pause = false;
                break;
            }
            // LevelEventPacket{LevelEvent::SimTimeStep, origin.getWorldPosition(), pause}.sendToClients();
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) mc->setSimTimePause(pause);
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

    tickCommand.runtimeOverload()
        .text("step")
        .required("time", ll::command::ParamKind::Int)
        .execute([&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            int tick = self["time"].get<ll::command::ParamKind::Int>();
            if (!::Command::validRange(tick, 0, INT_MAX, output)) {
                return;
            }
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) mc->mSimTimer.mSteppingTick = (float)tick;
            output.success("command.tick.step.output"_tr(tick));
        });
}
} // namespace coral_fans::commands
