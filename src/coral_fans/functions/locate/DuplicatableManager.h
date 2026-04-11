#include "bsci/GeometryGroup.h"
#include "coral_fans/Config.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>


namespace coral_fans::functions::locate {
class DuplicatableManager {
public:
    enum class ShowType : uint {
        Netherite    = 1 << 0,
        NetherSpring = 1 << 1,
        NetherFire   = 1 << 2,
        GlowStone    = 1 << 3,
        Mushroom     = 1 << 4,
        NetherGold   = 1 << 5,
        NetherQuartz = 1 << 6,
        NetherMagma  = 1 << 7,
        NetherGravel = 1 << 8,
        Blackstone   = 1 << 9,
        SoulSand     = 1 << 10,
    };

private:
    struct NetherData {
        std::map<BlockPos, std::unordered_set<BlockPos>> netheritePosMap;
        std::unordered_set<BlockPos>                     springPosSet;
        std::unordered_set<BlockPos>                     firePosMap;
        std::map<BlockPos, int>                          glowStonePosMap;
        std::map<BlockPos, bool>                         mushroomPosMap;
        std::unordered_set<BlockPos>                     netherGoldPosMap;
        std::unordered_set<BlockPos>                     netherQuartzPosMap;
        std::unordered_set<BlockPos>                     netherMagmaPosMap;
        std::unordered_set<BlockPos>                     netherGravelPosMap;
        std::unordered_set<BlockPos>                     blackstonePosMap;
        std::unordered_set<BlockPos>                     soulSandPosMap;
        bool                                             reload = false;
    };

    struct NetherThreadTemperaryData {
        LevelChunk*                  chunk = nullptr;
        NetherData                   threadData;
        bool                         worldBlockTargetShouldOperate = false;
        bool                         isEmpty                       = true;
        std::unordered_set<BlockPos> temperaryPoses;
        int                          temperaryInt = 0;
    };

    struct NetherBsciChunkData {
        bsci::GeometryGroup::GeoId netheriteGeoId    = {0};
        bsci::GeometryGroup::GeoId springGeoId       = {0};
        bsci::GeometryGroup::GeoId fireGeoId         = {0};
        bsci::GeometryGroup::GeoId glowStoneGeoId    = {0};
        bsci::GeometryGroup::GeoId mushroomGeoId     = {0};
        bsci::GeometryGroup::GeoId netherGoldGeoId   = {0};
        bsci::GeometryGroup::GeoId netherQuartzGeoId = {0};
        bsci::GeometryGroup::GeoId netherMagmaGeoId  = {0};
        bsci::GeometryGroup::GeoId netherGravelGeoId = {0};
        bsci::GeometryGroup::GeoId blackstoneGeoId   = {0};
        bsci::GeometryGroup::GeoId soulSandGeoId     = {0};

        int                        neighborValidCount       = 0;
        bool                       chunkSaved               = true;
        bsci::GeometryGroup::GeoId chunkSavedDrawGeoId      = {0};
        uint                       dataDrawed               = false;
        int                        runtimeRemoveTickCounter = 0;
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
    struct DuplicatableHook8;
    struct DuplicatableHook9;

private:
    void                       removeData();
    void                       draw();
    void                       netherDraw(BlockSource&, ChunkPos, NetherData&);
    void                       removeBsciData(ShowType);
    void                       bsciDataRuntimeRemove();
    bsci::GeometryGroup::GeoId drawNetherite(std::map<BlockPos, std::unordered_set<BlockPos>>&);
    bsci::GeometryGroup::GeoId drawSpring(std::unordered_set<BlockPos>&);
    bsci::GeometryGroup::GeoId drawFire(std::unordered_set<BlockPos>&);
    bsci::GeometryGroup::GeoId drawGlowStone(std::map<BlockPos, int>&);
    bsci::GeometryGroup::GeoId drawMushroom(std::map<BlockPos, bool>&);
    bsci::GeometryGroup::GeoId
         drawOre(std::unordered_set<BlockPos>&, config::Locate::DuplicatableOreStruct&, std::string, std::string);
    bool isChunkValid(BlockSource&, ChunkPos);
    void tryRemoveNetherChunkData(ChunkPos);
    void drawChunkSavedInfo(BlockSource&, ChunkPos, NetherBsciChunkData&);

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