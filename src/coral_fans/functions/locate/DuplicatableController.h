#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>


namespace coral_fans::functions::locate {

// decorationPostProcessChunk Hook 的通用逻辑宏
#define DUPLICATABLE_DECORATION_POST_PROCESS_CHUNK(ControllerType, ThreadDataType, DataType, neighborhood)             \
    do {                                                                                                               \
        auto            dim            = (neighborhood).mDimension;                                                    \
        DBChunkStorage* dbChunkStorage = static_cast<DBChunkStorage*>(&(*(dim)->mChunkSource->mOwnedParent));          \
        auto&           pos            = (neighborhood).mArea->mBounds.mMin;                                           \
        ChunkPos        originChunkPos = ChunkPos((pos)->x + 1, (pos)->z + 1);                                         \
        auto&           controller     = ControllerType::getInstance();                                                \
        for (int i = -1; i <= 1; i++) {                                                                                \
            for (int j = -1; j <= 1; j++) {                                                                            \
                ChunkPos chunkPos = originChunkPos + ChunkPos(i, j);                                                   \
                if (!dbChunkStorage->isChunkSaved(chunkPos)) {                                                         \
                    auto threadData   = std::make_unique<ThreadDataType>();                                            \
                    threadData->chunk = (neighborhood).getExistingChunk(originChunkPos).get();                         \
                    threadData->data  = std::make_unique<DataType>();                                                  \
                    auto threadId     = std::this_thread::get_id();                                                    \
                    {                                                                                                  \
                        std::lock_guard lock(controller.decorationThreadIdsLock);                                      \
                        controller.decorationThreadIds.emplace(threadId, std::move(threadData));                       \
                    }                                                                                                  \
                    auto ori = origin(neighborhood);                                                                   \
                    {                                                                                                  \
                        std::lock_guard lock(controller.decorationThreadIdsLock);                                      \
                        auto            it = controller.decorationThreadIds.find(threadId);                            \
                        if (it != controller.decorationThreadIds.end()) {                                              \
                            std::lock_guard lock2(controller.dataMapLock);                                             \
                            if (it->second->isEmpty) controller.dataMap.erase(originChunkPos);                         \
                            else {                                                                                     \
                                auto [newIter, inserted] = controller.dataMap.insert_or_assign(                        \
                                    originChunkPos,                                                                    \
                                    std::make_unique<DataType>(std::move(*static_cast<DataType*>(it->second->data.get( \
                                    ))))                                                                               \
                                );                                                                                     \
                                if (!inserted) newIter->second->reload = true;                                         \
                            }                                                                                          \
                            controller.decorationThreadIds.erase(it);                                                  \
                        }                                                                                              \
                    }                                                                                                  \
                    return ori;                                                                                        \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

class DuplicatableController {
public:
    // ========== 数据结构基类 ==========
    struct DataBase {
        bool reload         = false;
        virtual ~DataBase() = default;
    };

    struct ThreadTemporaryDataBase {
        LevelChunk*               chunk   = nullptr;
        bool                      isEmpty = true;
        std::unique_ptr<DataBase> data;
        virtual ~ThreadTemporaryDataBase() = default;
    };

    struct BsciChunkDataBase {
        int                        neighborValidCount       = 0;
        bool                       chunkSaved               = true;
        bsci::GeometryGroup::GeoId chunkSavedDrawGeoId      = {0};
        uint                       dataDrawed               = false;
        int                        runtimeRemoveTickCounter = 0;
        virtual ~BsciChunkDataBase()                        = default;
    };

protected:
    // ========== 通用数据成员 ==========
    std::mutex decorationThreadIdsLock;
    std::mutex dataMapLock;
    std::mutex bsciChunkDataLock;

    std::unordered_map<std::thread::id, std::unique_ptr<ThreadTemporaryDataBase>> decorationThreadIds;
    std::unordered_map<ChunkPos, std::unique_ptr<DataBase>>                       dataMap;
    std::unordered_map<ChunkPos, std::unique_ptr<BsciChunkDataBase>>              bsciChunkData;

    uint responsibleShowTypes = 0;

public:
    virtual ~DuplicatableController() = default;

    // ========== 纯虚接口 ==========
    virtual int getDimId() const = 0;

    // 绘制入口（由子类实现具体地物绘制）
    virtual void draw(BlockSource& region, ChunkPos chunkPos, uint showType) = 0;

    // 清理无效数据（removeData 中调用）
    void removeInvalidData(class ChunkSource& chunkSource);

    // 运行时移除 BSCI 数据
    void bsciDataRuntimeRemoveInternal(uint showType);

    // 按 ShowType 移除 BSCI 数据
    void removeBsciDataByType(uint showType);

    // 根据 ShowType 获取对应的 GeoId（由子类实现，不匹配时返回 nullptr）
    virtual bsci::GeometryGroup::GeoId* getGeoIdByShowType(BsciChunkDataBase& data, uint showType) = 0;

    // 判断某个 ShowType 是否属于该 Controller
    virtual bool isResponsibleFor(uint showType) const = 0;

    // 清除所有数据
    void clearInternal();

    // 移除指定 BsciChunkData 的所有 GeoId（由子类实现）
    virtual void removeAllGeoIds(BsciChunkDataBase& data) = 0;

    // 通用方法
    static bool isChunkValid(BlockSource& region, ChunkPos originChunkPos);

    // 尝试移除 Chunk 数据（通用逻辑）
    void tryRemoveChunkData(ChunkPos originChunkPos);

    // 绘制 Chunk 保存状态信息（通用逻辑）
    static void drawChunkSavedInfo(
        BlockSource&                                                      region,
        ChunkPos                                                          originChunkPos,
        BsciChunkDataBase&                                                originChunkData,
        std::unordered_map<ChunkPos, std::unique_ptr<BsciChunkDataBase>>& bsciChunkData
    );
};

} // namespace coral_fans::functions::locate
