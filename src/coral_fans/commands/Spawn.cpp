#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/commands/Commands.h"
#include "coral_fans/functions/minerule/MineruleManager.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"

#include <array>
#include <string_view>

namespace {
constexpr size_t kSpawnSurfaceIdx     = 1;
constexpr size_t kSpawnUndergroundIdx = 0;

template <size_t N>
auto getDimensionCaps(const float (&surfaceCaps)[N], const float (&undergroundCaps)[N]) {
    std::array<std::array<int, N>, 2> caps;
    for (size_t i = 0; i < N; ++i) {
        caps[kSpawnSurfaceIdx][i]     = static_cast<int>(surfaceCaps[i]);
        caps[kSpawnUndergroundIdx][i] = static_cast<int>(undergroundCaps[i]);
    }
    return caps;
}
} // namespace

namespace coral_fans::commands {
void registerSpawnCommand(config::CommandConfigStruct& config) {
    if (config.enabled) {
        using ll::i18n_literals::operator""_tr;
        auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                        .getOrCreateCommand(config.command, "command.spawn.description"_tr(), config.permission);

        // spawn count global
        cmd.runtimeOverload().text("count").text("global").execute([&](CommandOrigin const&,
                                                                       CommandOutput& output,
                                                                       ll::command::RuntimeCommand const&) {
            using ll::i18n_literals::operator""_tr;
            const auto& spawner = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
            const auto  cap     = functions::PopulationCapManager::getInstance().globalMax;
            return output.success("command.spawn.success.count.global"_tr(spawner.mSpawnableMobTickCountPrevious, cap));
        });

        // spawn count density base
        cmd.runtimeOverload().text("count").text("density").text("base").execute(
            [&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                COMMAND_CHECK_PLAYER
                auto& spawner = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
                spawner._updateBaseTypeCount(
                    player->getDimensionBlockSource(),
                    utils::blockPosToChunkPos(player->getFeetBlockPos())
                );
                const auto data = spawner.mBaseTypeCount;
                // [0] = underground, [1] = surface
                // [7] -> {Animal, Monster, WaterAnimal, Villager, Ambient, Cat, Pillager}

                const auto& dimension = player->getDimension();
                const auto  caps = getDimensionCaps(dimension.mMobsPerChunkSurface, dimension.mMobsPerChunkUnderground);
                // [0] = underground, [1] = surface

                static constexpr std::string_view categories[7] =
                    {"animal", "monster", "water_animal", "villager", "ambient", "cat", "pillager"};

                std::string result        = "command.spawn.success.count.basetype.title"_tr();
                auto        appendSection = [&](std::string_view labelKey, size_t index) {
                    auto label = ll::i18n::getInstance().get(
                        "command.spawn." + std::string(labelKey),
                        {}
                    );
                    result += "command.spawn.success.count.basetype.type"_tr(label);
                    for (size_t c = 0; c < 7; ++c) {
                        auto categoryStr = ll::i18n::getInstance().get(
                            "command.spawn.category." + std::string(categories[c]),
                            {}
                        );
                        result +=
                            "command.spawn.success.count.basetype.line"_tr(categoryStr, data[index][c], caps[index][c]);
                    }
                };
                appendSection("surface", kSpawnSurfaceIdx);
                appendSection("underground", kSpawnUndergroundIdx);
                return output.success(result);
            }
        );

        // spawn count density type
        cmd.runtimeOverload().text("count").text("density").text("type").execute(
            [&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                COMMAND_CHECK_PLAYER
                auto& spawner = static_cast<BedrockSpawner&>(ll::service::getLevel()->getSpawner());
                spawner._updateBaseTypeCount(
                    player->getDimensionBlockSource(),
                    utils::blockPosToChunkPos(player->getFeetBlockPos())
                );
                const auto& data = spawner.mEntityTypeCount;
                // [0] = underground, [1] = surface

                std::string result        = "command.spawn.success.count.type.title"_tr();
                auto        appendSection = [&](std::string_view labelKey, size_t index) {
                    bool first = true;
                    for (const auto& [type, count] : *data[index]) {
                        if (count == 0) continue;
                        if (first) {
                            auto label = ll::i18n::getInstance().get(
                                "command.spawn." + std::string(labelKey),
                                {}
                            );
                            result += "command.spawn.success.count.type.type"_tr(label);
                            first   = false;
                        }
                        result += "command.spawn.success.count.type.line"_tr(type.getString(), count);
                    }
                };
                appendSection("surface", kSpawnSurfaceIdx);
                appendSection("underground", kSpawnUndergroundIdx);
                return output.success(result);
            }
        );
    }
}
} // namespace coral_fans::commands