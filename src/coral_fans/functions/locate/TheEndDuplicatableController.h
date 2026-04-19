#pragma once

#include "DuplicatableController.h"
#include "mc/deps/core/math/Random.h"


namespace coral_fans::functions::locate {

class TheEndDuplicatableController : public DuplicatableController {
public:
    struct TheEndData : DataBase {
        std::multimap<BlockPos, Core::Random> endIslandPosMap;
        std::multimap<BlockPos, int>          chorusFlowerPosMap;
        std::unordered_set<BlockPos>          endGatewayPosSet;
    };

    struct TheEndThreadTemporaryData : ThreadTemporaryDataBase {
        int  temperaryInt    = 0;
        bool temperatureBool = true;
    };

    struct TheEndBsciChunkData : BsciChunkDataBase {
        bsci::GeometryGroup::GeoId endIslandGeoId    = {0};
        bsci::GeometryGroup::GeoId chorusFlowerGeoId = {0};
        bsci::GeometryGroup::GeoId endGatewayGeoId   = {0};
    };

private:
    struct TheEndHook1; // TheEndGenerator::$decorationPostProcessChunk
    struct TheEndHook2; // EndIslandFeature::$place
    struct TheEndHook3; // ChorusFlowerBlock::_growTreeRecursive
    struct TheEndHook4; // EndGatewayFeature::$place

public:
    [[nodiscard]] static TheEndDuplicatableController& getInstance();

    int getDimId() const override { return 2; }

    void                        draw(BlockSource& region, ChunkPos chunkPos, uint showType) override;
    bsci::GeometryGroup::GeoId* getGeoIdByShowType(BsciChunkDataBase& data, uint showType) override;
    bool                        isResponsibleFor(uint showType) const override;

    static void hook(bool enable);

private:
    TheEndDuplicatableController();

    bsci::GeometryGroup::GeoId drawEndIsland(std::multimap<BlockPos, Core::Random>& data);
    bsci::GeometryGroup::GeoId drawChorusFlower(std::multimap<BlockPos, int>& data);
    bsci::GeometryGroup::GeoId drawEndGateway(std::unordered_set<BlockPos>& data);

    void removeAllGeoIds(BsciChunkDataBase& data) override;
};

} // namespace coral_fans::functions::locate
