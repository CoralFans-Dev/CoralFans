#include "bsci/GeometryGroup.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include <memory>
#include <unordered_map>
#include <vector>


namespace coral_fans::functions::locate {
class DuplicatableManager {
private:
    struct NetherData {
        std::map<BlockPos, std::vector<BlockPos>> netheritePosMap;
        bool                                      reload = false;
    };

    struct NetherThreadTemperaryData {
        LevelChunk*           chunk = nullptr;
        NetherData            threadData;
        bool                  subChunkShouldOperate         = false;
        bool                  worldBlockTargetShouldOperate = false;
        bool                  isEmpty                       = true;
        std::vector<BlockPos> temeraryPoses;
    };

    struct NetherBsciChunkData {
        bsci::GeometryGroup::GeoId netheriteGeoId           = {0};
        int                        neighborValidCount       = 0;
        bool                       chunkSaved               = true;
        bsci::GeometryGroup::GeoId chunkSavedDrawGeoId      = {0};
        uint                       dataDrawed               = false;
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

    std::mutex                                                                      netherDecorationThreadIdsLock;
    std::unordered_map<std::thread::id, std::unique_ptr<NetherThreadTemperaryData>> netherDecorationThreadIds;

    std::mutex                               netherDataMapLock;
    std::unordered_map<ChunkPos, NetherData> netherDataMap;

    struct DuplicatableHook1;
    struct DuplicatableHook2;
    struct DuplicatableHook3;
    struct DuplicatableHook4;

private:
    void                       removeData();
    void                       draw();
    void                       netherDraw(BlockSource&, ChunkPos, NetherData&);
    void                       removeBsciData(ShowType);
    void                       bsciDataRuntimeRemove();
    bsci::GeometryGroup::GeoId drawNetherite(std::map<BlockPos, std::vector<BlockPos>>&);
    bool                       isChunkValid(BlockSource&, ChunkPos);
    void                       tryRemoveNetherChunkData(ChunkPos);
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