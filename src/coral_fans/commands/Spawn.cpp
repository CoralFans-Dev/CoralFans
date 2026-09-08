#include "coral_fans/functions/spawn/Spawn.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/commands/Commands.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/biome/MobSpawnerData.h"
#include "mc/world/level/biome/SpawnConditions.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/phys/HitResult.h"


#include <string_view>
#include <unordered_map>

namespace coral_fans::commands {
void registerSpawnCommand(config::CommandConfigStruct& config) {
    if (config.enabled) {
        using ll::i18n_literals::operator""_tr;
        auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                        .getOrCreateCommand(config.command, "command.spawn.description"_tr(), config.permission);

        // spawn count global
        cmd.runtimeOverload().text("count").text("global").execute(
            [&](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                const auto usage = functions::getSpawnableMobTickUsage();
                return output.success("command.spawn.success.count.global"_tr(usage.count, usage.cap));
            }
        );

        // spawn count density base
        cmd.runtimeOverload().text("count").text("density").text("base").execute(
            [&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                COMMAND_CHECK_PLAYER
                const auto density = functions::getBaseTypeDensity(
                    player->getDimensionBlockSource(),
                    utils::blockPosToChunkPos(player->getFeetBlockPos())
                );

                static constexpr std::string_view categories[7] =
                    {"animal", "monster", "water_animal", "villager", "ambient", "cat", "pillager"};

                std::string result        = "command.spawn.success.count.basetype.title"_tr();
                auto        appendSection = [&](std::string_view                    labelKey,
                                         const functions::MobCategoryCounts& counts,
                                         const functions::MobCategoryCounts& caps) {
                    auto label  = ll::i18n::getInstance().get("command.spawn." + std::string(labelKey), {});
                    result     += "command.spawn.success.count.basetype.type"_tr(label);
                    for (size_t c = 0; c < 7; ++c) {
                        auto categoryStr =
                            ll::i18n::getInstance().get("command.spawn.category." + std::string(categories[c]), {});
                        result += "command.spawn.success.count.basetype.line"_tr(
                            categoryStr,
                            counts.values[c],
                            caps.values[c]
                        );
                    }
                };
                appendSection("surface", density.count.surface, density.cap.surface);
                appendSection("underground", density.count.underground, density.cap.underground);
                return output.success(result);
            }
        );

        // spawn count density type
        cmd.runtimeOverload().text("count").text("density").text("type").execute(
            [&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                COMMAND_CHECK_PLAYER
                const auto counts = functions::getEntityTypeCounts(
                    player->getDimensionBlockSource(),
                    utils::blockPosToChunkPos(player->getFeetBlockPos())
                );

                std::string result        = "command.spawn.success.count.type.title"_tr();
                auto        appendSection = [&](std::string_view                               labelKey,
                                         const std::unordered_map<::HashedString, int>& section) {
                    bool first = true;
                    for (const auto& [type, count] : section) {
                        if (count == 0) continue;
                        if (first) {
                            auto label = ll::i18n::getInstance().get("command.spawn." + std::string(labelKey), {});
                            result += "command.spawn.success.count.type.type"_tr(label);
                            first   = false;
                        }
                        result += "command.spawn.success.count.type.line"_tr(type.getString(), count);
                    }
                };
                appendSection("surface", counts.surface);
                appendSection("underground", counts.underground);
                return output.success(result);
            }
        );

        // spawn prob
        cmd.runtimeOverload().text("prob").execute(
            [&](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
                using ll::i18n_literals::operator""_tr;
                COMMAND_CHECK_PLAYER
                const auto hitrst = player->traceRay(5.25f, false, true);
                if (hitrst.mType != HitResultType::Tile) return output.error("command.spawn.error.no_target"_tr());

                auto&          region = player->getDimensionBlockSource();
                const BlockPos pos    = hitrst.mBlock;

                SpawnConditions conditions;
                try {
                    conditions = functions::getSpawnConditions(region, pos);
                } catch (...) {
                    return output.error("command.spawn.error.position"_tr());
                }

                const auto candidates = functions::getCandidateMobs(region, pos, conditions);

                int totalWeight = 0;
                for (auto* mob : candidates) totalWeight += mob->mRandomWeight;

                std::string result  = "command.spawn.prob.title"_tr(pos.x, pos.y, pos.z);
                result             += "command.spawn.prob.brightness"_tr(conditions.rawBrightness);
                result             += "command.spawn.prob.surface"_tr(conditions.isOnSurface, conditions.isUnderground);
                result             += "command.spawn.prob.fluid"_tr(conditions.isInWater, conditions.isInLava);
                result             += "command.spawn.prob.biome"_tr(region.getBiome(pos).mHash.get().getString());

                for (auto* mob : candidates) {
                    const auto& identifier = *mob->mIdentifier;
                    bool        ok = ::Spawner::isSpawnPositionOk(*mob->mSpawnRules, region, utils::up(pos, 1), false);
                    double      prob  = totalWeight > 0 ? mob->mRandomWeight * 100.0 / totalWeight : 0.0;
                    result           += "command.spawn.prob.line"_tr(
                        identifier.mCanonicalName.get().getString(),
                        prob,
                        ok ? "command.spawn.prob.yes"_tr() : "command.spawn.prob.no"_tr()
                    );
                }
                return output.success(result);
            }
        );
    }
}
} // namespace coral_fans::commands