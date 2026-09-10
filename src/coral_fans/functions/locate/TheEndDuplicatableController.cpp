#include "TheEndDuplicatableController.h"
#include "DuplicatableManager.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/CoralFans.h"

#include "ll/api/base/StdInt.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "mc/deps/core/math/Color.h"
#include "mc/deps/core/math/Random.h"
#include "mc/util/Random.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/ChorusFlowerBlock.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/SubChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/feature/EndGatewayFeature.h"
#include "mc/world/level/levelgen/feature/EndIslandFeature.h"
#include "mc/world/level/levelgen/v1/TheEndGenerator.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>


namespace coral_fans::functions::locate {

// ========== Hooks ==========

LL_TYPE_INSTANCE_HOOK(
    TheEndDuplicatableController::TheEndHook1,
    ll::memory::HookPriority::Normal,
    TheEndGenerator,
    &TheEndGenerator::$decorationPostProcessChunk,
    bool,
    ChunkViewSource& neighborhood
) {
    DUPLICATABLE_DECORATION_POST_PROCESS_CHUNK(
        TheEndDuplicatableController,
        TheEndThreadTemporaryData,
        TheEndData,
        neighborhood
    );
    return origin(neighborhood);
}

LL_TYPE_INSTANCE_HOOK(
    TheEndDuplicatableController::TheEndHook2,
    ll::memory::HookPriority::Normal,
    EndIslandFeature,
    &EndIslandFeature::$place,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    int offsetX = pos.x % 16;
    int offsetZ = pos.z % 16;
    if ((offsetX == 8 || offsetX == 9) && (offsetZ == 8 || offsetZ == 9)) return origin(region, pos, random);
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = TheEndDuplicatableController::getInstance();
    TheEndThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end())
            threadData = static_cast<TheEndThreadTemporaryData*>(it->second.get());
    }
    if (!threadData) return origin(region, pos, random);
    Core::Random randomCopy = random.mRandom->mObject;
    int          radius     = randomCopy.nextInt(3) + 4;
    if (offsetX < 8 || offsetZ < 8 || offsetX + radius >= 16 || offsetZ + radius >= 16) {
        static_cast<TheEndData*>(threadData->data.get())->endIslandPosMap.emplace(pos, random.mRandom->mObject);
        threadData->isEmpty = false;
    }
    return origin(region, pos, random);
}

LL_TYPE_STATIC_HOOK(
    TheEndDuplicatableController::TheEndHook3,
    ll::memory::HookPriority::Normal,
    ChorusFlowerBlock,
    &ChorusFlowerBlock::_growTreeRecursive,
    void,
    ::BlockSource&    region,
    ::BlockPos const& current,
    ::BlockPos const& startPos,
    ::Random&         random,
    int               maxHorizontalSpread,
    int               depth
) {
    if (startPos.x % 16 >= 8 && startPos.z % 16 >= 8)
        return origin(region, current, startPos, random, maxHorizontalSpread, depth);
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = TheEndDuplicatableController::getInstance();
    TheEndThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end())
            threadData = static_cast<TheEndThreadTemporaryData*>(it->second.get());
    }
    if (threadData && threadData->temperatureBool) {
        static_cast<TheEndData*>(threadData->data.get())
            ->chorusFlowerPosMap.emplace(current, threadData->temperaryInt++);
        threadData->isEmpty         = false;
        threadData->temperatureBool = false;
        origin(region, current, startPos, random, maxHorizontalSpread, depth);
        threadData->temperatureBool = true;
    } else origin(region, current, startPos, random, maxHorizontalSpread, depth);
}

LL_TYPE_INSTANCE_HOOK(
    TheEndDuplicatableController::TheEndHook4,
    ll::memory::HookPriority::Normal,
    EndGatewayFeature,
    &EndGatewayFeature::$place,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    auto  threadId   = std::this_thread::get_id();
    auto& controller = TheEndDuplicatableController::getInstance();
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end()) {
            static_cast<TheEndData*>(it->second->data.get())->endGatewayPosSet.emplace(pos);
            it->second->isEmpty = false;
        }
    }
    return origin(region, pos, random);
}

// ========== 单例 ==========

TheEndDuplicatableController& TheEndDuplicatableController::getInstance() {
    static TheEndDuplicatableController instance;
    return instance;
}

TheEndDuplicatableController::TheEndDuplicatableController() {
    this->responsibleShowTypes = static_cast<uint>(DuplicatableManager::ShowType::EndIsland)
                               | static_cast<uint>(DuplicatableManager::ShowType::ChorusFlower)
                               | static_cast<uint>(DuplicatableManager::ShowType::EndGateway);
}

// ========== 绘制函数 ==========

bsci::GeometryGroup::GeoId TheEndDuplicatableController::drawEndIsland(std::multimap<BlockPos, Core::Random>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup   = CoralFans::getInstance().getGeometryGroup();
    auto& endIslandConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.endIsland;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(17 * data.size());
    for (auto& [pos, _random] : data) {
        auto random = _random;
        geoIdList.emplace_back(
            geometryGroup->box(2, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(endIslandConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            2,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            "translate.locate.duplicatable.endIsland.oriPos"_tr(),
            mce::Color(endIslandConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            2,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(endIslandConfig.arrowColor)
        ));
        float radius  = static_cast<float>(random.nextInt(3)) + 4;
        int   yOffset = 0;
        while (radius > 0.5f) {
            geoIdList.emplace_back(geometryGroup->box(
                2,
                AABB(
                    Vec3(pos.x - std::ceil(radius), pos.y - yOffset + 1, pos.z - std::ceil(radius)),
                    Vec3(pos.x + std::ceil(radius) + 1, pos.y - yOffset, pos.z + std::ceil(radius) + 1)
                ),
                mce::Color(endIslandConfig.boundColor)
            ));
            geoIdList.emplace_back(geometryGroup->cylinder(
                2,
                Vec3{pos.x + 0.5, pos.y - yOffset + 1, pos.z + 0.5},
                Vec3{pos.x + 0.5, pos.y - yOffset, pos.z + 0.5},
                radius + 1.0f,
                mce::Color(endIslandConfig.boundColor)
            ));
            yOffset++;
            radius -= static_cast<float>(random.nextInt(2)) + 0.5f;
        }
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId TheEndDuplicatableController::drawChorusFlower(std::multimap<BlockPos, int>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup      = CoralFans::getInstance().getGeometryGroup();
    auto& chorusFlowerConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.chorusFlower;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(3 * data.size());
    for (auto& [pos, number] : data) {
        geoIdList.emplace_back(
            geometryGroup->box(2, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(chorusFlowerConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            2,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            "translate.locate.duplicatable.chorusFlower.oriPos"_tr(number),
            mce::Color(chorusFlowerConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            2,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(chorusFlowerConfig.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId TheEndDuplicatableController::drawEndGateway(std::unordered_set<BlockPos>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup    = CoralFans::getInstance().getGeometryGroup();
    auto& endGatewayConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.endGateway;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(3 * data.size());
    for (auto& pos : data) {
        geoIdList.emplace_back(
            geometryGroup->box(2, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(endGatewayConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            2,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            "translate.locate.duplicatable.endGateway.oriPos"_tr(),
            mce::Color(endGatewayConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            2,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(endGatewayConfig.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

// ========== removeAllGeoIds ==========

void TheEndDuplicatableController::removeAllGeoIds(BsciChunkDataBase& data) {
    auto& theEndData    = static_cast<TheEndBsciChunkData&>(data);
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    if (theEndData.endIslandGeoId.value) {
        geometryGroup->remove(theEndData.endIslandGeoId);
        theEndData.endIslandGeoId.value = 0;
    }
    if (theEndData.chorusFlowerGeoId.value) {
        geometryGroup->remove(theEndData.chorusFlowerGeoId);
        theEndData.chorusFlowerGeoId.value = 0;
    }
    if (theEndData.endGatewayGeoId.value) {
        geometryGroup->remove(theEndData.endGatewayGeoId);
        theEndData.endGatewayGeoId.value = 0;
    }
}

// ========== 接口实现 ==========

void TheEndDuplicatableController::draw(BlockSource& region, ChunkPos chunkPos, uint showType) {
    using ll::i18n_literals::operator""_tr;

    std::lock_guard lock(this->dataMapLock);
    auto            it = this->dataMap.find(chunkPos);
    if (it == this->dataMap.end()) {
        std::lock_guard lock2(this->bsciChunkDataLock);
        this->tryRemoveChunkData(chunkPos);
        return;
    }

    auto& data = *static_cast<TheEndData*>(it->second.get());

    bool hasData =
        (showType & static_cast<uint>(DuplicatableManager::ShowType::EndIsland) && !data.endIslandPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::ChorusFlower)
            && !data.chorusFlowerPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::EndGateway) && !data.endGatewayPosSet.empty());

    if (!hasData) return;

    if (!DuplicatableController::isChunkValid(region, chunkPos)) {
        this->dataMap.erase(it);
        std::lock_guard lock2(this->bsciChunkDataLock);
        this->tryRemoveChunkData(chunkPos);
        return;
    }

    std::lock_guard lock2(this->bsciChunkDataLock);
    auto [bsciIt, inserted] = this->bsciChunkData.try_emplace(chunkPos, std::make_unique<TheEndBsciChunkData>());
    auto& bsciData          = *static_cast<TheEndBsciChunkData*>(bsciIt->second.get());
    if (!bsciData.dataDrawed)
        DuplicatableController::drawChunkSavedInfo<TheEndBsciChunkData>(region, chunkPos, bsciData, this->bsciChunkData);
    else if (data.reload) {
        this->removeAllGeoIds(bsciData);
        bsciData.dataDrawed = 0;
    }
    bsciData.runtimeRemoveTickCounter = 0;
    data.reload                       = false;

    if (showType & static_cast<uint>(DuplicatableManager::ShowType::EndIsland) && data.endIslandPosMap.size()
        && !bsciData.endIslandGeoId.value) {
        bsciData.endIslandGeoId  = this->drawEndIsland(data.endIslandPosMap);
        bsciData.dataDrawed     |= static_cast<uint>(DuplicatableManager::ShowType::EndIsland);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::ChorusFlower) && data.chorusFlowerPosMap.size()
        && !bsciData.chorusFlowerGeoId.value) {
        bsciData.chorusFlowerGeoId  = this->drawChorusFlower(data.chorusFlowerPosMap);
        bsciData.dataDrawed        |= static_cast<uint>(DuplicatableManager::ShowType::ChorusFlower);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::EndGateway) && !data.endGatewayPosSet.empty()
        && !bsciData.endGatewayGeoId.value) {
        bsciData.endGatewayGeoId  = drawEndGateway(data.endGatewayPosSet);
        bsciData.dataDrawed      |= static_cast<uint>(DuplicatableManager::ShowType::EndGateway);
    }
}

bsci::GeometryGroup::GeoId* TheEndDuplicatableController::getGeoIdByShowType(BsciChunkDataBase& data, uint showType) {
    auto& theEndData = static_cast<TheEndBsciChunkData&>(data);
    switch (static_cast<DuplicatableManager::ShowType>(showType)) {
    case DuplicatableManager::ShowType::EndIsland:
        return &theEndData.endIslandGeoId;
    case DuplicatableManager::ShowType::ChorusFlower:
        return &theEndData.chorusFlowerGeoId;
    case DuplicatableManager::ShowType::EndGateway:
        return &theEndData.endGatewayGeoId;
    default:
        return nullptr;
    }
}

bool TheEndDuplicatableController::isResponsibleFor(uint showType) const {
    return showType & this->responsibleShowTypes;
}

void TheEndDuplicatableController::hook(bool enable) {
    if (enable) {
        auto& duplicatableConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable;
        bool  shouldHook         = false;
        if (duplicatableConfig.endIsland.enable) {
            TheEndHook2::hook();
            shouldHook = true;
        } else TheEndHook2::unhook();
        if (duplicatableConfig.chorusFlower.enable) {
            TheEndHook3::hook();
            shouldHook = true;
        } else TheEndHook3::unhook();
        if (duplicatableConfig.endGateway.enable) {
            TheEndHook4::hook();
            shouldHook = true;
        } else TheEndHook4::unhook();
        if (shouldHook) TheEndHook1::hook();
        else TheEndHook1::unhook();
    } else {
        TheEndHook1::unhook();
        TheEndHook2::unhook();
        TheEndHook3::unhook();
        TheEndHook4::unhook();
    }
}

} // namespace coral_fans::functions::locate
