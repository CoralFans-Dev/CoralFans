#pragma once

#include "DuplicatableController.h"
#include "coral_fans/Config.h"


namespace coral_fans::functions::locate {

class NetherDuplicatableController : public DuplicatableController {
public:
    // ========== 子类数据结构 ==========
    struct NetherData : DataBase {
        std::multimap<BlockPos, std::unordered_set<BlockPos>> netheritePosMap;
        std::unordered_set<BlockPos>                          springPosSet;
        std::unordered_set<BlockPos>                          firePosMap;
        std::multimap<BlockPos, int>                          glowStonePosMap;
        std::multimap<BlockPos, bool>                         mushroomPosMap;
        std::unordered_set<BlockPos>                          netherGoldPosMap;
        std::unordered_set<BlockPos>                          netherQuartzPosMap;
        std::unordered_set<BlockPos>                          netherMagmaPosMap;
        std::unordered_set<BlockPos>                          netherGravelPosMap;
        std::unordered_set<BlockPos>                          blackstonePosMap;
        std::unordered_set<BlockPos>                          soulSandPosMap;
    };

    struct NetherThreadTemporaryData : ThreadTemporaryDataBase {
        bool                         worldBlockTargetShouldOperate = false;
        std::unordered_set<BlockPos> temperaryPoses;
        int                          temperaryInt = 0;
    };

    struct NetherBsciChunkData : BsciChunkDataBase {
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
        int                        temperaryInt      = 0;
    };

private:
    // Hook 声明
    struct NetherHook1; // NetherGenerator::$decorationPostProcessChunk
    struct NetherHook2; // NoSurfaceOreFeature::$place
    struct NetherHook3; // WorldBlockTarget::$getBlock
    struct NetherHook4; // IFeature::isExposedTo
    struct NetherHook5; // NetherSpringFeature::$place
    struct NetherHook6; // NetherFireFeature::$place
    struct NetherHook7; // GlowStoneFeature::$place
    struct NetherHook8; // MushroomFeature::$place
    struct NetherHook9; // OreFeature::$place

public:
    [[nodiscard]] static NetherDuplicatableController& getInstance();

    int getDimId() const override { return 1; }

    void                        draw(BlockSource& region, ChunkPos chunkPos, uint showType) override;
    bsci::GeometryGroup::GeoId* getGeoIdByShowType(BsciChunkDataBase& data, uint showType) override;
    bool                        isResponsibleFor(uint showType) const override;

    // Hook 控制
    static void hook(bool enable);

private:
    NetherDuplicatableController();

    // 具体绘制函数
    bsci::GeometryGroup::GeoId drawNetherite(std::multimap<BlockPos, std::unordered_set<BlockPos>>& data);
    bsci::GeometryGroup::GeoId drawSpring(std::unordered_set<BlockPos>& data);
    bsci::GeometryGroup::GeoId drawFire(std::unordered_set<BlockPos>& data);
    bsci::GeometryGroup::GeoId drawGlowStone(std::multimap<BlockPos, int>& data);
    bsci::GeometryGroup::GeoId drawMushroom(std::multimap<BlockPos, bool>& data);
    bsci::GeometryGroup::GeoId
    drawOre(std::unordered_set<BlockPos>& data, config::Locate::DuplicatableOreStruct& config, std::string text);

    void removeAllGeoIds(BsciChunkDataBase& data) override;
};

} // namespace coral_fans::functions::locate
