#include "coral_fans/functions/spawn/SpawnAnalyzer.h"

#include "mc/deps/game_refs/GameRefs.h"
#include "mc/deps/ecs/gamerefs_entity/GameRefsEntity.h"

#include "coral_fans/base/Utils.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/world/ServerLevelTickEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/i18n/I18n.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"


#include <cstdlib>

namespace coral_fans::functions {

SpawnAnalyzer& SpawnAnalyzer::getInstance() {
    static SpawnAnalyzer instance;
    return instance;
}

bool SpawnAnalyzer::start(Player& player) {
    if (mAnalyzing) return false;
    clear();
    mDimensionId    = player.getDimensionId();
    mCenterChunkPos = utils::blockPosToChunkPos(player.getFeetBlockPos());

    auto& bus         = ll::event::EventBus::getInstance();
    auto  mobListener = bus.emplaceListener<ll::event::world::SpawnedMobEvent>(
        [this](ll::event::world::SpawnedMobEvent& event) { onMobSpawned(event); }
    );
    if (!mobListener) return false;
    auto tickListener = bus.emplaceListener<ll::event::world::ServerLevelTickEvent>(
        [this](ll::event::world::ServerLevelTickEvent& event) { onLevelTick(event.level()); }
    );
    if (!tickListener) {
        bus.removeListener(mobListener);
        return false;
    }
    mListeners.emplace_back(std::move(mobListener));
    mListeners.emplace_back(std::move(tickListener));
    mAnalyzing = true;
    return true;
}

bool SpawnAnalyzer::stop() {
    if (!mAnalyzing) return false;
    unsubscribe();
    mAnalyzing = false;
    return true;
}

void SpawnAnalyzer::clear() {
    mTickCount = 0;
    mSurfaceSpawnCounts.clear();
    mCaveSpawnCounts.clear();
    mSurfaceDensitySamples.clear();
    mCaveDensitySamples.clear();
}

void SpawnAnalyzer::shutdown() {
    unsubscribe();
    mAnalyzing = false;
    clear();
}

void SpawnAnalyzer::unsubscribe() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto const& listener : mListeners) bus.removeListener(listener);
    mListeners.clear();
}

void SpawnAnalyzer::onMobSpawned(ll::event::world::SpawnedMobEvent& event) {
    if (!mAnalyzing) return;
    auto mob = event.mob();
    if (!mob) return;
    if (mob->getDimensionId() != mDimensionId) return;
    ++(event.surface() ? mSurfaceSpawnCounts : mCaveSpawnCounts)[mob->getTypeName()];
}

void SpawnAnalyzer::onLevelTick(Level& level) {
    if (!mAnalyzing) return;
    ++mTickCount;
    for (const auto& entity : level.getEntities()) {
        if (!entity) continue;
        const auto* actor = entity.tryUnwrap<Actor>().as_ptr();
        if (!actor || actor->getDimensionId() != mDimensionId) continue;
        if (actor->isType(ActorType::Player)) continue;
        const auto actorChunk = utils::blockPosToChunkPos(actor->getFeetBlockPos());
        if (std::abs(actorChunk.x - mCenterChunkPos.x) > 4 || std::abs(actorChunk.z - mCenterChunkPos.z) > 4) {
            continue;
        }
        ++(actor->isSurfaceMob() ? mSurfaceDensitySamples : mCaveDensitySamples)[actor->getTypeName()];
    }
}

std::optional<std::string> SpawnAnalyzer::buildResult() const {
    using ll::i18n_literals::operator""_tr;
    if (mTickCount == 0) return std::nullopt;

    const double hours = static_cast<double>(mTickCount) / 72000.0;
    std::string result = "command.spawn.analyze.print.title"_tr(mTickCount, hours);

    auto appendSection = [&](const std::string&                    labelKey,
                             const std::map<std::string, uint64_t>& densities,
                             const std::map<std::string, uint64_t>& spawns) {
        result += ll::i18n::getInstance().get(labelKey, {});
        bool any = false;
        for (const auto& [type, samples] : densities) {
            if (type == "minecraft:player") continue;
            any     = true;
            result += "command.spawn.analyze.print.line"_tr(
                type,
                static_cast<double>(samples) / static_cast<double>(mTickCount)
            );
            if (auto iter = spawns.find(type); iter != spawns.end()) {
                result += "command.spawn.analyze.print.spawn"_tr(iter->second, iter->second / hours);
            }
        }
        if (!any) result += "command.spawn.analyze.print.empty"_tr();
    };
    appendSection("command.spawn.analyze.print.surface", mSurfaceDensitySamples, mSurfaceSpawnCounts);
    appendSection("command.spawn.analyze.print.underground", mCaveDensitySamples, mCaveSpawnCounts);
    return result;
}

} // namespace coral_fans::functions
