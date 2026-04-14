#include "coral_fans/functions/minerule/MineruleManager.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>

namespace coral_fans::commands {

void registerPopcapCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& popcapCommand = ll::command::CommandRegistrar::getInstance(false)
                              .getOrCreateCommand("popcap", "command.popcap.description"_tr(), permission);

    // 注册子类型枚举 (surface, underground)
    ll::command::CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(
        "popcapType",
        {
            {"surface",     0},
            {"underground", 1}
    }
    );

    // 注册种群类别枚举
    ll::command::CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(
        "mobTypes",
        {
            {"animal",       0},
            {"monster",      1},
            {"water_animal", 2},
            {"villager",     3},
            {"ambient",      4},
            {"cat",          5},
            {"pillager",     6}
    }
    );

    // popcap global <count>
    popcapCommand.runtimeOverload()
        .text("global")
        .required("count", ll::command::ParamKind::Int)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            int count = self["count"].get<ll::command::ParamKind::Int>();
            // 这里不需要处理负数，游戏默认负数不限制
            functions::PopulationCapManager::getInstance().setGlobalMax(count);
            output.success("command.popcap.global.success"_tr(count));
        });

    // popcap global reset
    popcapCommand.runtimeOverload().text("global").text("reset").execute(
        [](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
            functions::PopulationCapManager::getInstance().setGlobalMax(200);
            output.success("command.popcap.global.reset.success"_tr());
        }
    );

    // popcap dim <dimension> <mobtype> <type> <count>
    popcapCommand.runtimeOverload()
        .required("dimension", ll::command::ParamKind::Dimension)
        .required("mobtype", ll::command::ParamKind::Enum, "mobTypes")
        .required("type", ll::command::ParamKind::Enum, "popcapType")
        .required("count", ll::command::ParamKind::Float)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto  dimId       = self["dimension"].get<ll::command::ParamKind::Dimension>();
            int   category    = static_cast<int>(self["mobtype"].get<ll::command::ParamKind::Enum>().index);
            bool  isOnSurface = self["type"].get<ll::command::ParamKind::Enum>().index == 0;
            float count       = self["count"].get<ll::command::ParamKind::Float>();

            // 如果用户输入负数，他的意思很可能是想取消这个类别的限制，所以我们把它改成非常大
            // 由于浮点误差，最大可取值是2147483583.0f，超过这个值不刷怪
            if (count < 0 || count > 2147483583.0f) count = 2147483583.0f;

            auto dimKey =
                "translate.dimension." + std::to_string(self["dimension"].get<ll::command::ParamKind::Dimension>());
            auto dimStr      = ll::i18n::getInstance().get(dimKey, {});
            auto typeStr     = isOnSurface ? "command.surface"_tr() : "command.underground"_tr();
            auto categoryKey = "command.popcap.category." + self["mobtype"].get<ll::command::ParamKind::Enum>().name;
            auto categoryStr = ll::i18n::getInstance().get(categoryKey, {});

            if (functions::PopulationCapManager::getInstance().setDimCap(dimId, category, isOnSurface, count)) {
                output.success("command.popcap.dim.success"_tr(dimStr, categoryStr, typeStr, count));
            } else {
                output.error("command.popcap.dim.error"_tr(dimStr, categoryStr, typeStr));
            }
        });

    // popcap dim <dimension> reset
    popcapCommand.runtimeOverload()
        .required("dimension", ll::command::ParamKind::Dimension)
        .text("reset")
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto dimId = self["dimension"].get<ll::command::ParamKind::Dimension>();
            auto dimKey =
                "translate.dimension." + std::to_string(self["dimension"].get<ll::command::ParamKind::Dimension>());
            auto dimStr = ll::i18n::getInstance().get(dimKey, {});

            if (functions::PopulationCapManager::getInstance().resetDimCap(dimId)) {
                output.success("command.popcap.dim.reset.success"_tr(dimStr));
            } else {
                output.error("command.popcap.dim.reset.error"_tr(dimStr));
            }
        });

    functions::PopulationCapManager::getInstance().init();
}
} // namespace coral_fans::commands
