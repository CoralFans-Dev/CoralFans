#include "coral_fans/CoralFans.h"
#include "coral_fans/functions/minerule/MineruleManager.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"


namespace coral_fans::commands {
void registerMineruleCommand(config::CommandConfigStruct& config) {
    if (config.enabled) {
        using ll::i18n_literals::operator""_tr;

        auto& mineruleCommand = ll::command::CommandRegistrar::getInstance(false).getOrCreateCommand(
            config.command,
            "command.minerule.description"_tr(),
            config.permission
        );

        static constexpr std::string_view dims[3] = {"overworld", "nether", "the_end"};

        mineruleCommand.runtimeOverload()
            .text("fuck_bedrock_no_drop")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (isopen) {
                    if (CoralFans::getInstance().getConfigDb()->set("minerule.bedrockDrop", "true")) {
                        output.success("command.minerule.bedrockDrop.success.true"_tr());
                        functions::bedrockDropHook(true);
                    } else output.error("command.minerule.bedrockDrop.error"_tr());
                } else {
                    if (CoralFans::getInstance().getConfigDb()->set("minerule.bedrockDrop", "false")) {
                        output.success("command.minerule.bedrockDrop.success.false"_tr());
                        functions::bedrockDropHook(false);
                    } else output.error("command.minerule.bedrockDrop.error"_tr());
                }
            });

        mineruleCommand.runtimeOverload()
            .text("fuck_movingBlock_no_drop")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (isopen) {
                    if (CoralFans::getInstance().getConfigDb()->set("minerule.movingBlockDrop", "true")) {
                        output.success("command.minerule.movingBlockDrop.success.true"_tr());
                        functions::mbDropHook(true);
                    } else output.error("command.minerule.movingBlockDrop.error"_tr());
                } else {
                    if (CoralFans::getInstance().getConfigDb()->set("minerule.movingBlockDrop", "false")) {
                        output.success("command.minerule.movingBlockDrop.success.false"_tr());
                        functions::mbDropHook(false);
                    } else output.error("command.minerule.movingBlockDrop.error"_tr());
                }
            });

        mineruleCommand.runtimeOverload()
            .text("restore_portal_sand_farm")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (CoralFans::getInstance().getConfigDb()->set(
                        "minerule.restore_portal_sand_farm",
                        isopen ? "true" : "false"
                    )) {
                    output.success("command.minerule.restore_portal_sand_farm.success"_tr(isopen ? "true" : "false"));
                    functions::portalSandFarmHook(isopen);
                } else output.error("command.minerule.restore_portal_sand_farm.error"_tr());
            });

        mineruleCommand.runtimeOverload()
            .text("remove_portal_pigzombie_cd")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (CoralFans::getInstance().getConfigDb()->set(
                        "minerule.remove_portal_pigzombie_cd",
                        isopen ? "true" : "false"
                    )) {
                    output.success("command.minerule.remove_portal_pigzombie_cd.success"_tr(isopen ? "true" : "false"));
                    functions::portalSpawnHook(isopen);
                } else output.error("command.minerule.remove_portal_pigzombie_cd.error"_tr());
            });

        mineruleCommand.runtimeOverload()
            .text("restore_ancillary_broken")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (CoralFans::getInstance().getConfigDb()->set(
                        "minerule.restore_ancillary_broken",
                        isopen ? "true" : "false"
                    )) {
                    output.success("command.minerule.restore_ancillary_broken.success"_tr(isopen ? "true" : "false"));
                    functions::restoreAncillaryBrokenHook(isopen);
                } else output.error("command.minerule.restore_ancillary_broken.error"_tr());
            });

        mineruleCommand.runtimeOverload()
            .text("fuck_population_cap")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (CoralFans::getInstance().getConfigDb()->set(
                        "minerule.fuck_population_cap",
                        isopen ? "true" : "false"
                    )) {
                    output.success("command.minerule.fuck_population_cap.success"_tr(isopen ? "true" : "false"));
                    functions::populationCapHook(isopen);
                } else output.error("command.minerule.fuck_population_cap.error"_tr());
            });

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

        // minerule popcap global <count>
        mineruleCommand.runtimeOverload()
            .text("popcap")
            .text("global")
            .required("count", ll::command::ParamKind::Int)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                int count = self["count"].get<ll::command::ParamKind::Int>();
                // 这里不需要处理负数，游戏默认负数不限制
                functions::PopulationCapManager::getInstance().setGlobalMax(count);
                output.success("command.minerule.popcap.global.success"_tr(count));
            });

        // minerule popcap global reset
        mineruleCommand.runtimeOverload().text("popcap").text("global").text("reset").execute(
            [](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
                functions::PopulationCapManager::getInstance().setGlobalMax(200);
                output.success("command.minerule.popcap.global.reset.success"_tr());
            }
        );

        // minerule popcap dim <dimension> <mobtype> <type> <count>
        mineruleCommand.runtimeOverload()
            .text("popcap")
            .required("dimension", ll::command::ParamKind::Dimension)
            .required("mobtype", ll::command::ParamKind::Enum, "mobTypes")
            .required("type", ll::command::ParamKind::Enum, "popcapType")
            .required("count", ll::command::ParamKind::Float)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                int dimId = self["dimension"].get<ll::command::ParamKind::Dimension>().id;
                if (dimId < 0 || dimId > 2) {
                    output.error("Invalid dimension");
                    return;
                }

                int   category    = static_cast<int>(self["mobtype"].get<ll::command::ParamKind::Enum>().index);
                bool  isOnSurface = self["type"].get<ll::command::ParamKind::Enum>().index == 0;
                float count       = self["count"].get<ll::command::ParamKind::Float>();

                // 如果用户输入负数，他的意思很可能是想取消这个类别的限制，所以我们把它改成非常大
                // 由于浮点误差，最大可取值是2147483583.0f，超过这个值不刷怪
                if (count < 0 || count > 2147483583.0f) count = 2147483583.0f;

                auto dimKey  = "translate.dimension." + std::string(dims[dimId]);
                auto dimStr  = ll::i18n::getInstance().get(dimKey, {});
                auto typeKey = "command.minerule.popcap." + self["type"].get<ll::command::ParamKind::Enum>().name;
                auto typeStr = ll::i18n::getInstance().get(typeKey, {});
                auto categoryKey =
                    "command.minerule.popcap.category." + self["mobtype"].get<ll::command::ParamKind::Enum>().name;
                auto categoryStr = ll::i18n::getInstance().get(categoryKey, {});

                if (functions::PopulationCapManager::getInstance().setDimCap(dimId, category, isOnSurface, count)) {
                    output.success("command.minerule.popcap.dim.success"_tr(dimStr, categoryStr, typeStr, count));
                } else {
                    output.error("command.minerule.popcap.dim.error"_tr(dimStr, categoryStr, typeStr));
                }
            });

        // minerule popcap dim <dimension> reset
        mineruleCommand.runtimeOverload()
            .text("popcap")
            .required("dimension", ll::command::ParamKind::Dimension)
            .text("reset")
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                int dimId = self["dimension"].get<ll::command::ParamKind::Dimension>().id;
                if (dimId < 0 || dimId > 2) {
                    output.error("Invalid dimension");
                    return;
                }

                auto dimKey = "translate.dimension." + std::string(dims[dimId]);
                auto dimStr = ll::i18n::getInstance().get(dimKey, {});

                if (functions::PopulationCapManager::getInstance().resetDimCap(dimId)) {
                    output.success("command.minerule.popcap.dim.reset.success"_tr(dimStr));
                } else {
                    output.error("command.minerule.popcap.dim.reset.error"_tr(dimStr));
                }
            });

        functions::PopulationCapManager::getInstance().init();

        mineruleCommand.runtimeOverload()
            .text("fuck_piston_reset_velocity")
            .required("isopen", ll::command::ParamKind::Bool)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
                if (CoralFans::getInstance().getConfigDb()->set(
                        "minerule.fuck_piston_reset_velocity",
                        isopen ? "true" : "false"
                    )) {
                    output.success("command.minerule.fuck_piston_reset_velocity.success"_tr(isopen ? "true" : "false"));
                    functions::pistonCollisionHook(isopen);
                } else {
                    output.error("command.minerule.fuck_piston_reset_velocity.error"_tr());
                }
            });

        // minerule set_max_pt_consume_per_chunk <int>
        mineruleCommand.runtimeOverload()
            .text("set_max_pt_consume_per_chunk")
            .required("maxpt", ll::command::ParamKind::Int)
            .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
                using ll::i18n_literals::operator""_tr;
                int maxpt = self["maxpt"].get<ll::command::ParamKind::Int>();
                if (maxpt <= 0) output.error("command.func.maxpt.error.nonpositive"_tr());
                if (CoralFans::getInstance().getConfigDb()->set("functions.global.maxpt", std::to_string(maxpt))) {
                    functions::MaxPtManager::getInstance().maxpt = maxpt;
                    functions::MaxPtManager::hook(maxpt != 100);
                    output.success("command.func.maxpt.success"_tr(maxpt));
                } else output.error("command.func.maxpt.error.failed"_tr());
            });
        functions::MaxPtManager::getInstance().maxpt =
            std::stoi(CoralFans::getInstance().getConfigDb()->get("functions.global.maxpt").value_or("100"));
    }

    auto& configDb = CoralFans::getInstance().getConfigDb();

    functions::bedrockDropHook(config.enabled && configDb->get("minerule.bedrockDrop") == "true");

    functions::mbDropHook(config.enabled && configDb->get("minerule.movingBlockDrop") == "true");

    functions::portalSandFarmHook(config.enabled && configDb->get("minerule.restore_portal_sand_farm") == "true");

    functions::portalSpawnHook(config.enabled && configDb->get("minerule.remove_portal_pigzombie_cd") == "true");

    functions::restoreAncillaryBrokenHook(
        config.enabled && configDb->get("minerule.restore_ancillary_broken") == "true"
    );

    functions::populationCapHook(config.enabled && configDb->get("minerule.fuck_population_cap") == "true");

    functions::pistonCollisionHook(config.enabled && configDb->get("minerule.fuck_piston_reset_velocity") == "true");

    functions::MaxPtManager::hook(config.enabled && functions::MaxPtManager::getInstance().maxpt != 100);
}
} // namespace coral_fans::commands
