#include "bsci/GeometryGroup.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include <unordered_map>


namespace coral_fans::functions::locate {
class DuplicatableManager {
private:
    struct BlockPosPairHash {
        size_t operator()(const std::pair<BlockPos, BlockPos>& p) const {
            // 使用 Boost 的 hash_combine 或自定义组合
            size_t h1 = std::hash<BlockPos>()(p.first);
            size_t h2 = std::hash<BlockPos>()(p.second);

            return h1 ^ (h2 << 1);
        }
    };

    struct NetherData {
        std::unordered_set<std::pair<BlockPos, BlockPos>, BlockPosPairHash> netheritePosSet;
    };

    struct NetherBsciChunkData {
        bsci::GeometryGroup::GeoId netheriteGeoId           = {0};
        int                        neighborValidCount       = 0;
        bool                       chunkSaved               = true;
        bsci::GeometryGroup::GeoId chunkSavedDrawGeoId      = {0};
        bool                       dataDrawed               = false;
        int                        runtimeRemoveTickCounter = 0;
    };

public:
    enum class ShowType : uint {
        Netherite = 1 << 0,
    };

private:
    uint showType                   = 0;
    int  tickCounter                = 1;
    int  cacheDataRemoveTickCounter = 1;

    std::unordered_map<ChunkPos, NetherBsciChunkData> netherBsciChunkData;

    std::mutex                                       netherDecorationThreadIdsLock;
    std::unordered_map<std::thread::id, LevelChunk*> netherDecorationThreadIds;

    std::mutex                               netherDataMapLock;
    std::unordered_map<ChunkPos, NetherData> netherDataMap;

    struct DuplicatableHook1;
    struct DuplicatableHook2;
    struct DuplicatableHook3;

private:
    void                       removeData();
    void                       draw();
    void                       removeBsciData(ShowType);
    void                       bsciDataRuntimeRemove();
    bsci::GeometryGroup::GeoId drawNetherite(std::unordered_set<std::pair<BlockPos, BlockPos>, BlockPosPairHash>&);
    bool                       isChunkValid(BlockSource&, ChunkPos);
    void                       removeNetherChunkData(ChunkPos);
    void                       drawChunkSavedInfo(BlockSource&, ChunkPos, NetherBsciChunkData&);

public:
    void tick();

public:
    void        setShowType(ShowType, bool);
    bool        getShowType(ShowType);
    std::string test(ChunkPos);

public:
    static DuplicatableManager& getInstance() {
        static DuplicatableManager instance;
        return instance;
    }
    static void hook(bool);
};
} // namespace coral_fans::functions::locate