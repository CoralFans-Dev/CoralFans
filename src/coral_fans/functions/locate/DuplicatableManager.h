#include "bsci/GeometryGroup.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>


namespace coral_fans::functions::locate {
class DuplicatableManager {
private:
    struct NetherData {
        std::map<BlockPos, std::unordered_set<BlockPos>> netheritePosMap;
        std::unordered_set<BlockPos>                     springPosSet;
        std::unordered_set<BlockPos>                     firePosMap;
        std::unordered_map<BlockPos, int>                glowStonePosMap;
        bool                                             reload = false;
    };

    struct NetherThreadTemperaryData {
        LevelChunk*                  chunk = nullptr;
        NetherData                   threadData;
        bool                         subChunkShouldOperate         = false;
        bool                         worldBlockTargetShouldOperate = false;
        bool                         blockSourceShouldOperate      = false;
        bool                         isEmpty                       = true;
        std::unordered_set<BlockPos> temperaryPoses;
        int                          temperaryInt  = 0;
        bool                         temperaryBool = false;
    };

    struct NetherBsciChunkData {
        bsci::GeometryGroup::GeoId netheriteGeoId = {0};
        bsci::GeometryGroup::GeoId springGeoId    = {0};
        bsci::GeometryGroup::GeoId fireGeoId      = {0};
        bsci::GeometryGroup::GeoId glowStoneGeoId = {0};

        int                        neighborValidCount       = 0;
        bool                       chunkSaved               = true;
        bsci::GeometryGroup::GeoId chunkSavedDrawGeoId      = {0};
        uint                       dataDrawed               = false;
        int                        runtimeRemoveTickCounter = 0;
    };

public:
    enum class ShowType : uint {
        Netherite    = 1 << 0,
        NetherSpring = 1 << 1,
        NetherFire   = 1 << 2,
        GlowStone    = 1 << 3,
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
    struct DuplicatableHook5;
    struct DuplicatableHook6;
    struct DuplicatableHook7;

private:
    void                       removeData();
    void                       draw();
    void                       netherDraw(BlockSource&, ChunkPos, NetherData&);
    void                       removeBsciData(ShowType);
    void                       bsciDataRuntimeRemove();
    bsci::GeometryGroup::GeoId drawNetherite(std::map<BlockPos, std::unordered_set<BlockPos>>&);
    bsci::GeometryGroup::GeoId drawSpring(std::unordered_set<BlockPos>&);
    bsci::GeometryGroup::GeoId drawFire(std::unordered_set<BlockPos>&);
    bsci::GeometryGroup::GeoId drawGlowStone(std::unordered_map<BlockPos, int>&);
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