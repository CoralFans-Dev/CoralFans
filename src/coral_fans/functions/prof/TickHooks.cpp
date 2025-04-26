#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"

#include "ll/api/memory/Hook.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/BlockTickingQueue.h"
#include "mc/world/level/EntitySystemsManager.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/TickNextTickData.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/redstone/circuit/CircuitSceneGraph.h"
#include <string>


namespace coral_fans::functions {

// main game tick
LL_TYPE_INSTANCE_HOOK(CoralFansTickLevelTickHook, ll::memory::HookPriority::Normal, Level, &Level::$tick, void) {
    auto& mod  = coral_fans::mod();
    auto& prof = mod.getProfiler();
    PROF_TIMER(level, { origin(); })
    PROF_TIMER(coralfans, { mod.tick(); })
    if (prof.profiling) {
        prof.gameSessionTickTime += (time_level + time_coralfans);
        prof.gameSessionTicksBuffer.push_back(time_level + time_coralfans);
        prof.coralfansSessionTickTime += time_coralfans;
        prof.currentRound++;
        if (prof.currentRound == prof.totalRound) {
            prof.stop();
        }
    }
}

// LevelChunk tick
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickLevelChunkTickHook,
    ll::memory::HookPriority::Normal,
    LevelChunk,
    &LevelChunk::tick,
    void,
    BlockSource& tickRegion,
    Tick const&  tick
) {
    auto&      prof     = coral_fans::mod().getProfiler();
    const auto dimid    = tickRegion.getDimensionId();
    auto&      chunkPos = this->mPosition;
    if (prof.profiling) {
        PROF_TIMER(chunk, { origin(tickRegion, tick); })
        prof.chunkInfo.totalTickTime += time_chunk;
        prof.chunkInfo.chunk_counter[static_cast<int>(dimid)][chunkPos].push_back(time_chunk);
    } else origin(tickRegion, tick);
}

// LevelChunk tickBlocks
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickLevelChunkTickBlocksHook,
    ll::memory::HookPriority::Normal,
    LevelChunk,
    &LevelChunk::tickBlocks,
    void,
    BlockSource& region
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(chunk_block, { origin(region); })
        prof.chunkInfo.randomTickTime += time_chunk_block;
    } else origin(region);
}

// LevelChunk tickBlockEntities
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickLevelChunkTickBlockEntitiesHook,
    ll::memory::HookPriority::Normal,
    LevelChunk,
    &LevelChunk::tickBlockEntities,
    void,
    BlockSource& tickRegion
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(chunk_block_entity, { origin(tickRegion); })
        prof.chunkInfo.blockEntitiesTickTime += time_chunk_block_entity;
    } else origin(tickRegion);
}

// BlockTickingQueue tickPendingTicks
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickBlockTickingQueueTickPendingTicksHook,
    ll::memory::HookPriority::Normal,
    BlockTickingQueue,
    &BlockTickingQueue::tickPendingTicks,
    bool,
    BlockSource& region,
    Tick const&  until,
    int          max,
    bool         instaTick_
) {
    max        = coral_fans::functions::MaxPtManager::getInstance().maxpt;
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        bool res;
        // from
        // https://github.com/glibcxx/figure_hack/blob/f74b0badc2a2397f811282a3cdda3725f7e13c55/src/figure_hack/Function/PendingTickVisualization.cpp#L56
        // auto mNextTickQueue = ll::memory::dAccess<std::vector<BlockTickingQueue::BlockTick>>(this, 16);
        auto _mNextTickQueue = this->mNextTickQueue->mC;
        if (!_mNextTickQueue.empty()) {
            auto              tickData      = _mNextTickQueue.front().mData;
            TickNextTickData* _tickData     = (TickNextTickData*)&tickData;
            auto              chunkPos      = utils::blockPosToChunkPos(_tickData->pos);
            auto              dimId         = static_cast<int>(region.getDimensionId());
            auto              current       = prof.ptCounter[dimId][chunkPos];
            prof.ptCounter[dimId][chunkPos] = std::max(current, _mNextTickQueue.size());
        }
        PROF_TIMER(chunk_pt, { res = origin(region, until, max, instaTick_); })
        prof.chunkInfo.pendingTickTime += time_chunk_pt;
        return res;
    } else return origin(region, until, max, instaTick_);
}

// Dimension tick
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickDimensionTickHook,
    ll::memory::HookPriority::Normal,
    Dimension,
    &Dimension::$tick,
    void
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(dimension, { origin(); })
        prof.dimensionTickTime += time_dimension;
    } else origin();
}

// EntitySystems tick
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickEntitySystemsTickHook,
    ll::memory::HookPriority::Normal,
    EntitySystemsManager,
    &EntitySystemsManager::tickEntitySystems,
    void
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(entity, { origin(); })
        prof.entitySystemTickTime += time_entity;
    } else origin();
}

// redstone stuff

// signal update
// Dimension tickRedstone
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickDimensionTickRedstoneHook,
    ll::memory::HookPriority::Normal,
    Dimension,
    &Dimension::$tickRedstone,
    void
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(dimension_redstone, { origin(); })
        prof.redstoneInfo.signalUpdate += time_dimension_redstone;
    } else origin();
}

// pending add
// CircuitSceneGraph processPendingAdds
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickCircuitSceneGraphProcessPendingAddsHook,
    ll::memory::HookPriority::Normal,
    CircuitSceneGraph,
    &CircuitSceneGraph::processPendingAdds,
    void
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(pt_add, { origin(); })
        prof.redstoneInfo.pendingAdd += time_pt_add;
    } else origin();
}

// pending update
// CircuitSceneGraph processPendingUpdates
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickCircuitSceneGraphProcessPendingUpdatesHook,
    ll::memory::HookPriority::Normal,
    CircuitSceneGraph,
    &CircuitSceneGraph::update,
    void,
    BlockSource* region
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(pt_update, { origin(region); })
        prof.redstoneInfo.pendingUpdate += time_pt_update;
    } else origin(region);
}

// pending remove
// CircuitSceneGraph removeComponent
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickCircuitSceneGraphRemoveComponentHook,
    ll::memory::HookPriority::Normal,
    CircuitSceneGraph,
    &CircuitSceneGraph::removeComponent,
    void,
    BlockPos const& pos
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        PROF_TIMER(pt_remove, { origin(pos); })
        prof.redstoneInfo.pendingRemove += time_pt_remove;
    } else return origin(pos);
}

// redstone stuff end

// Actor tick
LL_TYPE_INSTANCE_HOOK(
    CoralFansTickActorTickHook,
    ll::memory::HookPriority::Normal,
    Actor,
    &Actor::tick,
    bool,
    BlockSource& region
) {
    auto& prof = coral_fans::mod().getProfiler();
    if (prof.profiling) {
        bool res;
        PROF_TIMER(actor, { res = origin(region); })
        prof.actorInfo[static_cast<int>(this->getDimensionId())][this->getNameTag() + " (" + this->getTypeName() + ")"]
            .time += time_actor;
        ++(prof.actorInfo[static_cast<int>(this->getDimensionId())]
                         [this->getNameTag() + " (" + this->getTypeName() + ")"]
                             .count);
        return res;
    } else return origin(region);
}

void hookTick(bool hook) {
    if (hook) {
        CoralFansTickLevelTickHook::hook();
        CoralFansTickLevelChunkTickHook::hook();
        CoralFansTickLevelChunkTickBlocksHook::hook();
        CoralFansTickLevelChunkTickBlockEntitiesHook::hook();
        coral_fans::functions::MaxPtManager::getInstance().maxpt =
            std::stoi(coral_fans::mod().getConfigDb()->get("functions.global.maxpt").value_or("100"));
        CoralFansTickBlockTickingQueueTickPendingTicksHook::hook();
        CoralFansTickDimensionTickHook::hook();
        CoralFansTickEntitySystemsTickHook::hook();
        CoralFansTickDimensionTickRedstoneHook::hook();
        CoralFansTickCircuitSceneGraphProcessPendingAddsHook::hook();
        CoralFansTickCircuitSceneGraphProcessPendingUpdatesHook::hook();
        CoralFansTickCircuitSceneGraphRemoveComponentHook::hook();
        CoralFansTickActorTickHook::hook();
    } else {
        CoralFansTickLevelTickHook::unhook();
        CoralFansTickLevelChunkTickHook::unhook();
        CoralFansTickLevelChunkTickBlocksHook::unhook();
        CoralFansTickLevelChunkTickBlockEntitiesHook::unhook();
        CoralFansTickBlockTickingQueueTickPendingTicksHook::unhook();
        CoralFansTickDimensionTickHook::unhook();
        CoralFansTickEntitySystemsTickHook::unhook();
        CoralFansTickDimensionTickRedstoneHook::unhook();
        CoralFansTickCircuitSceneGraphProcessPendingAddsHook::unhook();
        CoralFansTickCircuitSceneGraphProcessPendingUpdatesHook::unhook();
        CoralFansTickCircuitSceneGraphRemoveComponentHook::unhook();
        CoralFansTickActorTickHook::unhook();
    }
}

} // namespace coral_fans::functions