#include "LocateManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/feature/NoSurfaceOreFeature.h"
#include "mc/world/level/levelgen/v1/NetherGenerator.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <vector>


namespace coral_fans::functions::locate {
LL_TYPE_INSTANCE_HOOK(
    LocateManager::DuplicatableHook1,
    ll::memory::HookPriority::Normal,
    NetherGenerator,
    &NetherGenerator ::$decorationPostProcessChunk,
    bool,
    ChunkViewSource& neighborhood
) {
    auto            dim            = neighborhood.mDimension;
    DBChunkStorage* dbChunkStorage = static_cast<DBChunkStorage*>(&(*dim->mChunkSource->mOwnedParent));
    auto&           pos            = neighborhood.mArea->mBounds.mMin;
    ChunkPos        originChunkPos = ChunkPos(pos->x + 1, pos->z + 1);
    auto&           region         = dim->getBlockSourceFromMainChunkSource();
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) {
                ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);
                auto     chunk    = region.getChunk(chunkPos);
                if (chunk->isNonActorDataDirty() && !dbChunkStorage->isChunkSaved(chunkPos)) {
                    auto& locateManager = LocateManager::getInstance();
                    auto  threadId      = std::this_thread::get_id();
                    {
                        std::lock_guard lock(locateManager.netherDecorationThreadIdsLock);
                        locateManager.netherDecorationThreadIds.push_back(threadId);
                    }
                    auto ori = origin(neighborhood);
                    {
                        std::lock_guard lock(locateManager.netherDecorationThreadIdsLock);
                        std::erase_if(locateManager.netherDecorationThreadIds, [threadId](const std::thread::id& id) {
                            return id == threadId;
                        });
                    }
                    return ori;
                }
            }
        }
    }
    return origin(neighborhood);
}

LL_TYPE_INSTANCE_HOOK(
    LocateManager::DuplicatableHook2,
    ll::memory::HookPriority::Normal,
    NoSurfaceOreFeature,
    &NoSurfaceOreFeature ::$place,
    std::optional<::BlockPos>,
    IFeature::PlacementContext const& context
) {
    auto ori = origin(context);
    if (!ori.has_value()) {
        auto  threadId      = std::this_thread::get_id();
        auto& locateManager = LocateManager::getInstance();
        {
            std::lock_guard lock(locateManager.netherDecorationThreadIdsLock);
            if (std::find(
                    locateManager.netherDecorationThreadIds.begin(),
                    locateManager.netherDecorationThreadIds.end(),
                    threadId
                )
                == locateManager.netherDecorationThreadIds.end()) {
                return ori;
            }
        }
        ChunkPos chunkPos = ChunkPos(*context.mPos);
        if (ChunkPos(ori.value()) != chunkPos) {
            std::lock_guard lock(locateManager.ancientDebrisPosMapLock);
            locateManager.ancientDebrisPosMap.emplace(chunkPos, std::make_pair(*context.mPos, ori.value()));
        }
    }
    return ori;
}

void LocateManager::hook(bool enable) {
    if (enable) {
        DuplicatableHook1::hook();
        DuplicatableHook2::hook();
    } else {
        DuplicatableHook1::unhook();
        DuplicatableHook2::unhook();
    }
}
} // namespace coral_fans::functions::locate