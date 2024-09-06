#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/service/Bedrock.h"
// #include "mc/network/packet/LevelEventPacket.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/util/ProfilerLite.h"
#include "mc/util/Timer.h"
#include "mc/world/Minecraft.h"

#include "magic_enum.hpp"

namespace coral_fans::commands {
void registerTickCommand() {
    // reg cmd
    auto& tickCommand = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "tick",
        "Controls or queries the tick status of the game.",
        CommandPermissionLevel::GameDirectors
    );

    // tick query
    tickCommand.overload().text("query").execute([](CommandOrigin const&, CommandOutput& output) {
        output.success(
            "{} per tick",
            std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                ProfilerLite::gProfilerLiteInstance.getServerTickTime()
            )
        );
    });

    // tick freeze|reset
    enum class TickFreezeType { reset = 0, freeze = 1 };
    struct TickFreezeParam {
        TickFreezeType tickFreezeType;
    };
    tickCommand.overload<TickFreezeParam>()
        .required("tickFreezeType")
        .execute([&](CommandOrigin const&, CommandOutput& output, TickFreezeParam const& param) {
            bool pause = false;
            switch (param.tickFreezeType) {
            case TickFreezeType::freeze:
                pause = true;
                break;
            case TickFreezeType::reset:
                pause = false;
                break;
            }
            // LevelEventPacket{LevelEvent::SimTimeStep, origin.getWorldPosition(), pause}.sendToClients();
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) mc->setSimTimePause(pause);
            output.success("set tick {}", magic_enum::enum_name(param.tickFreezeType));
        });

    // tick rate <float>
    struct TickRateParam {
        float rate;
    };
    tickCommand.overload<TickRateParam>().text("rate").required("rate").execute(
        [](CommandOrigin const&, CommandOutput& output, TickRateParam const& param) {
            if (param.rate < 0) output.error("rate must be non negative");
            // LevelEventPacket{LevelEvent::SimTimeScale, {param.rate / 20}, param.rate > 0}.sendToClients();
            auto mc = ll::service::getMinecraft();
            if (mc.has_value()) mc->setSimTimeScale(param.rate / 20.0f);
            output.success(" set {} tick(s) per second", param.rate);
        }
    );

    // tick step <int>
    static auto stepFn = [](CommandOutput& output, int tick) {
        if (!::Command::validRange(tick, 0, INT_MAX, output)) {
            return;
        }
        auto mc = ll::service::getMinecraft();
        if (mc.has_value()) {
            // Minecraft.mSimTimer
            auto* timer = ll::memory::dAccess<Timer*>(mc.as_ptr(), 0xD8); // this+0xd8
            timer->stepTick(tick);
        }
        output.success("step {} tick(s)", tick);
    };
    struct TickStepParam {
        int time;
    };
    tickCommand.overload<TickStepParam>().text("step").required("time").execute(
        [&](CommandOrigin const&, CommandOutput& output, TickStepParam const& param) { stepFn(output, param.time); }
    );
    tickCommand.overload<TickStepParam>().text("step").required("time").postfix("t").execute(
        [&](CommandOrigin const&, CommandOutput& output, TickStepParam const& param) { stepFn(output, param.time); }
    );
    tickCommand.overload<TickStepParam>().text("step").required("time").postfix("s").execute(
        [&](CommandOrigin const&, CommandOutput& output, TickStepParam const& param) {
            stepFn(output, param.time * 20);
        }
    );
    tickCommand.overload<TickStepParam>().text("step").required("time").postfix("d").execute(
        [&](CommandOrigin const&, CommandOutput& output, TickStepParam const& param) {
            stepFn(output, param.time * 24000);
        }
    );
}
} // namespace coral_fans::commands
