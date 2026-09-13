#include "Commands.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/MySchedule.h"
#include "coral_fans/base/Utils.h"


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
#include "mc/world/level/Level.h"

#include "coral_fans/CoralFans.h"
#include "coral_fans/functions/tick/Tick.h"
#include "mc/deps/shared_types/legacy/LevelEvent.h"
#include "mc/network/packet/LevelEventPacket.h"
#include "mc/network/packet/UpdateAbilitiesPacket.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"


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
            if (mc.has_value()) {
                if (val.index) {
                    mc->setSimTimePause(true);
                    output.success("command.tick.set.freeze"_tr(val.name));
                } else {
                    LevelEventPacket pkt;
                    pkt.mEventId = static_cast<int>(SharedTypes::Legacy::LevelEvent::SimTimeScale);
                    pkt.mPos->x  = 1.0f;
                    pkt.sendToClients();
                    if (auto level = ll::service::getLevel()) {
                        level->forEachPlayer([](Player& player) {
                            player.getAbilities().setAbility(AbilitiesIndex::FlySpeed, 0.05f);
                            UpdateAbilitiesPacket{player.getOrCreateUniqueID(), player.getAbilities()}.sendTo(player);
                            return true;
                        });
                    }
                    mc->setSimTimePause(false);
                    mc->setSimTimeScale(1.0f);
                    output.success("command.tick.set.reset"_tr(val.name));
                }
            }
        });

    // tick rate <float>
    tickCommand.runtimeOverload()
        .text("rate")
        .required("rate", ll::command::ParamKind::Float)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            float rate = self["rate"].get<ll::command::ParamKind::Float>();
            if (rate < 0) output.error("command.tick.rate.error.outofrange"_tr());
            LevelEventPacket pkt;
            pkt.mEventId = static_cast<int>(SharedTypes::Legacy::LevelEvent::SimTimeScale);
            pkt.mPos->x  = 1.0f;
            pkt.sendToClients();
            pkt.mPos->x = rate / 20.0f;
            pkt.sendToClients();
            auto mc = ll::service::getMinecraft();
            if (auto level = ll::service::getLevel()) {
                level->forEachPlayer([rate](Player& player) {
                    player.getAbilities().setAbility(AbilitiesIndex::FlySpeed, 0.05f * 20.0f / rate);
                    UpdateAbilitiesPacket{player.getOrCreateUniqueID(), player.getAbilities()}.sendTo(player);
                    return true;
                });
            }
            mc->setSimTimePause(false);
            if (mc.has_value()) {
                mc->setSimTimePause(false);
                mc->setSimTimeScale(rate / 20.0f);
                output.success("command.tick.rate.success"_tr(rate));
            } else output.error("command.tick.rate.error.generic"_tr());
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
                if (auto mc = ll::service::getMinecraft(); mc && mc->mSimTimer.mSteppingTick <= 1e-4) {
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
            if (mc.has_value()) mc->mSimTimer.mSteppingTick = (float)tick;
            output.success("command.tick.step.output"_tr(tick));
        });

    functions::TickHook(true);
}
} // namespace coral_fans::commands
