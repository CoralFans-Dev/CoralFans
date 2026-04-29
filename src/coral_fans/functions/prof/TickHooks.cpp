#include "coral_fans/base/Macros.h"
#include "coral_fans/base/MySchedule.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/func/FuncManager.h"
#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/functions/hud/Hud.h"
#include "coral_fans/functions/locate/DuplicatableManager.h"
#include "coral_fans/functions/prof/Prof.h"
#include "coral_fans/functions/slime/Slime.h"
#include "coral_fans/functions/village/Village.h"


#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/profile/ProfilerLite.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/BlockTickingQueue.h"
#include "mc/world/level/EntitySystemsManager.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/TickNextTickData.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/redstone/circuit/CircuitSceneGraph.h"


#ifdef LL_PLAT_C
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


namespace coral_fans::functions {

// main game tick
LL_TYPE_INSTANCE_HOOK(CoralFansTickLevelTickHook, ll::memory::HookPriority::Normal, Level, &Level::$tick, void) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    auto& prof = functions::Profiler::getInstance();
    origin();
    auto time_level = ProfilerLite::gProfilerLiteInstance().mDebugServerTickTime->count() / 1000;
    PROF_TIMER(coralfans, {
        functions::HopperCounterManager::getInstance().tick();
        functions::HsaManager::getInstance().tick();                             // heavy 60
        functions::SlimeManager::getInstance().tick();                           // heavy 60
        functions::CFVillageManager::getInstance().tick(this->getCurrentTick()); // light 10
        functions::HudHelper::getInstance().tick();                              // light 20
        my_schedule::MySchedule::getSchedule().update();
        functions::locate::DuplicatableManager::getInstance().tick();
    })
    if (prof.profiling) {
        prof.gameSessionTickTime += time_level;
        prof.gameSessionTicksBuffer.push_back(time_level);
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
    &LevelChunk::tickImpl,
    void,
    BlockSource&            tickRegion,
    Tick const&             tick,
    ::std::function<void()> spawnerCallback
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(tickRegion, tick, spawnerCallback);
#endif
    auto&      prof     = functions::Profiler::getInstance();
    const auto dimid    = tickRegion.getDimensionId();
    auto&      chunkPos = this->mPosition;
    if (prof.profiling) {
        PROF_TIMER(chunk, { origin(tickRegion, tick, spawnerCallback); })
        prof.chunkInfo.totalTickTime += time_chunk;
        prof.chunkInfo.chunk_counter[static_cast<int>(dimid)][chunkPos].push_back(time_chunk);
    } else origin(tickRegion, tick, spawnerCallback);
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region);
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(tickRegion);
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region, until, max, instaTick_);
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region);
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(pos);
#endif
    auto& prof = functions::Profiler::getInstance();
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
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region);
#endif
    auto& prof = functions::Profiler::getInstance();
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

void hookTick(bool hook, bool disable) {
    if (!disable) CoralFansTickLevelTickHook::hook();
    else CoralFansTickLevelTickHook::unhook();
    if (hook) {
        CoralFansTickLevelChunkTickHook::hook();
        CoralFansTickLevelChunkTickBlocksHook::hook();
        CoralFansTickLevelChunkTickBlockEntitiesHook::hook();
        CoralFansTickBlockTickingQueueTickPendingTicksHook::hook();
        CoralFansTickDimensionTickHook::hook();
        CoralFansTickEntitySystemsTickHook::hook();
        CoralFansTickDimensionTickRedstoneHook::hook();
        CoralFansTickCircuitSceneGraphProcessPendingAddsHook::hook();
        CoralFansTickCircuitSceneGraphProcessPendingUpdatesHook::hook();
        CoralFansTickCircuitSceneGraphRemoveComponentHook::hook();
        CoralFansTickActorTickHook::hook();
    } else {
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