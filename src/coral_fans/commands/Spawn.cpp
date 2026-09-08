#include "coral_fans/CoralFans.h"
#include "coral_fans/commands/Commands.h"
#include "coral_fans/functions/minerule/MineruleManager.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"

#include <array>
#include <string_view>

namespace {
template <size_t N>
auto getDimensionCaps(const float (&surfaceCaps)[N], const float (&undergroundCaps)[N]) {
    std::array<std::array<int, N>, 2> caps;
    for (size_t i = 0; i < N; ++i) {
        caps[0][i] = static_cast<int>(surfaceCaps[i]);
        caps[1][i] = static_cast<int>(undergroundCaps[i]);
    }
    return caps;
}
} // namespace

BedrockSpawner* defaultSpawner = nullptr;
LL_TYPE_INSTANCE_HOOK(
    getDefaultSpawnerHook,
    HookPriority::Normal,
    BedrockSpawner,
    &BedrockSpawner::$tickMobCount,
    void
) {
    origin();
    if (!defaultSpawner) {
        defaultSpawner = this;
        coral_fans::CoralFans::getInstance().getSelf().getLogger().info(
            "Default spawner set to: {}",
            static_cast<void*>(defaultSpawner)
        );
        getDefaultSpawnerHook::unhook();
    }
}

namespace coral_fans::commands {
void registerSpawnCommand(config::CommandConfigStruct& config) {
    if (config.enabled) {
        using ll::i18n_literals::operator""_tr;
        const auto& logger = CoralFans::getInstance().getSelf().getLogger();

        auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                        .getOrCreateCommand(config.command, "command.spawn.description"_tr(), config.permission);

        // spawn count global
        cmd.runtimeOverload().text("count").text("global").execute([&](CommandOrigin const&,
                                                                       CommandOutput& output,
                                                                       ll::command::RuntimeCommand const&) {
            using ll::i18n_literals::operator""_tr;
            const auto& spawner    = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
            const auto  spawnerPtr = static_cast<const void*>(&spawner);
            const auto  defaultPtr = static_cast<void*>(defaultSpawner);
            logger.info(
                "this spawner: {}, default spawner: {}, isSame: {}",
                spawnerPtr,
                defaultPtr,
                spawnerPtr == defaultPtr ? "true" : "false"
            );
            const auto cap = functions::PopulationCapManager::getInstance().globalMax;
            return output.success("command.spawn.success.count.global"_tr(spawner.mSpawnableMobTickCountPrevious, cap));
        });

        // spawn count basetype
        cmd.runtimeOverload()
            .text("count")
            .text("basetype")
            .execute([&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                const auto& spawner    = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
                const auto  spawnerPtr = static_cast<const void*>(&spawner);
                const auto  defaultPtr = static_cast<void*>(defaultSpawner);
                logger.info(
                    "this spawner: {}, default spawner: {}, isSame: {}",
                    spawnerPtr,
                    defaultPtr,
                    spawnerPtr == defaultPtr ? "true" : "false"
                );
                const auto& data = spawner.mBaseTypeCount;
                // [2][7] -> {Surface, Underground} x {Animal, Monster, WaterAnimal, Villager, Ambient, Cat, Pillager}
                auto* dimension = origin.getDimension();
                if (!dimension) return output.error("command.spawn.error.no_dimension"_tr());
                const auto caps =
                    getDimensionCaps(dimension->mMobsPerChunkSurface, dimension->mMobsPerChunkUnderground);

                static constexpr std::string_view categories[7] =
                    {"animal", "monster", "water_animal", "villager", "ambient", "cat", "pillager"};
                static constexpr std::string_view typeKeys[2] = {"surface", "underground"};

                std::string result = "command.spawn.success.count.basetype.title"_tr();
                for (size_t t = 0; t < 2; ++t) {
                    auto typeStr = ll::i18n::getInstance().get(
                        "command.minerule.fuck_population_cap." + std::string(typeKeys[t]),
                        {}
                    );
                    result += "command.spawn.success.count.basetype.type"_tr(typeStr);
                    for (size_t c = 0; c < 7; ++c) {
                        auto categoryStr = ll::i18n::getInstance().get(
                            "command.minerule.fuck_population_cap.category." + std::string(categories[c]),
                            {}
                        );
                        result += "command.spawn.success.count.basetype.line"_tr(categoryStr, data[t][c], caps[t][c]);
                    }
                }
                return output.success(result);
            });

        // spawn count type
        cmd.runtimeOverload().text("count").text("type").execute(
            [&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                const auto& spawner    = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
                const auto  spawnerPtr = static_cast<const void*>(&spawner);
                const auto  defaultPtr = static_cast<void*>(defaultSpawner);
                logger.info(
                    "this spawner: {}, default spawner: {}, isSame: {}",
                    spawnerPtr,
                    defaultPtr,
                    spawnerPtr == defaultPtr ? "true" : "false"
                );
                const auto& data = spawner.mEntityTypeCount;

                static constexpr std::string_view typeKeys[2] = {"surface", "underground"};

                std::string result = "command.spawn.success.count.type.title"_tr();
                for (size_t t = 0; t < 2; ++t) {
                    bool first = true;
                    for (const auto& [type, count] : *data[t]) {
                        if (count == 0) continue;
                        if (first) {
                            auto typeStr = ll::i18n::getInstance().get(
                                "command.minerule.fuck_population_cap." + std::string(typeKeys[t]),
                                {}
                            );
                            result += "command.spawn.success.count.type.type"_tr(typeStr);
                            first   = false;
                        }
                        result += "command.spawn.success.count.type.line"_tr(type.getString(), count);
                    }
                }
                return output.success(result);
            }
        );

        getDefaultSpawnerHook::hook();
    }
}
} // namespace coral_fans::commands