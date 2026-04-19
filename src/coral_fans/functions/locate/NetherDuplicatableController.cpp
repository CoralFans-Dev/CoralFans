#include "NetherDuplicatableController.h"
#include "DuplicatableManager.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/CoralFans.h"

#include "ll/api/base/StdInt.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "mc/deps/core/math/Color.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/WorldBlockTarget.h"
#include "mc/world/level/chunk/ChunkViewSource.h"
#include "mc/world/level/chunk/SubChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/feature/GlowStoneFeature.h"
#include "mc/world/level/levelgen/feature/MushroomFeature.h"
#include "mc/world/level/levelgen/feature/NetherFireFeature.h"
#include "mc/world/level/levelgen/feature/NetherSpringFeature.h"
#include "mc/world/level/levelgen/feature/NoSurfaceOreFeature.h"
#include "mc/world/level/levelgen/feature/OreFeature.h"
#include "mc/world/level/levelgen/v1/NetherGenerator.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace coral_fans::functions::locate {

// ========== Hooks ==========

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook1,
    ll::memory::HookPriority::Normal,
    NetherGenerator,
    &NetherGenerator::$decorationPostProcessChunk,
    bool,
    ChunkViewSource& neighborhood
) {
    DUPLICATABLE_DECORATION_POST_PROCESS_CHUNK(
        NetherDuplicatableController,
        NetherThreadTemporaryData,
        NetherData,
        neighborhood
    );
    return origin(neighborhood);
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook2,
    ll::memory::HookPriority::Normal,
    NoSurfaceOreFeature,
    &NoSurfaceOreFeature::$place,
    std::optional<::BlockPos>,
    IFeature::PlacementContext const& context
) {
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = NetherDuplicatableController::getInstance();
    NetherThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end())
            threadData = static_cast<NetherThreadTemporaryData*>(it->second.get());
    }
    if (!threadData) return origin(context);
    threadData->worldBlockTargetShouldOperate = true;
    auto ori                                  = origin(context);
    threadData->worldBlockTargetShouldOperate = false;
    if (!threadData->temperaryPoses.empty()) {
        static_cast<NetherData*>(threadData->data.get())
            ->netheritePosMap.emplace(context.mPos, std::move(threadData->temperaryPoses));
        threadData->isEmpty = false;
    }
    return ori;
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook3,
    ll::memory::HookPriority::Normal,
    WorldBlockTarget,
    &WorldBlockTarget::$getBlock,
    ::Block const&,
    ::BlockPos const& pos
) {
    auto&                      ori        = origin(pos);
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = NetherDuplicatableController::getInstance();
    NetherThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it == controller.decorationThreadIds.end()
            || !static_cast<NetherThreadTemporaryData*>(it->second.get())->worldBlockTargetShouldOperate)
            return ori;
        else threadData = static_cast<NetherThreadTemporaryData*>(it->second.get());
    }
    auto chunkPos = ChunkPos(pos);
    int  offsetX  = threadData->chunk->mPosition->x - chunkPos.x;
    int  offsetZ  = threadData->chunk->mPosition->z - chunkPos.z;
    if (offsetX || offsetZ) {
        BlockPos checkPos = BlockPos(pos.x + offsetX, pos.y, pos.z + offsetZ);
        if (ChunkPos(checkPos) != threadData->chunk->mPosition || !origin(checkPos).isAir())
            threadData->temperaryPoses.emplace(pos);
    }
    return std::forward<decltype(ori)>(ori);
}

LL_TYPE_STATIC_HOOK(
    NetherDuplicatableController::NetherHook4,
    ll::memory::HookPriority::Normal,
    IFeature,
    &IFeature::isExposedTo,
    bool,
    ::IBlockWorldGenAPI const& target,
    ::BlockPos const&          candidatePos,
    ::BlockDescriptor const&   exposedTo
) {
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = NetherDuplicatableController::getInstance();
    NetherThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end()
            && static_cast<NetherThreadTemporaryData*>(it->second.get())->worldBlockTargetShouldOperate)
            threadData = static_cast<NetherThreadTemporaryData*>(it->second.get());
    }
    if (!threadData) return origin(target, candidatePos, exposedTo);
    threadData->worldBlockTargetShouldOperate = false;
    auto ori                                  = origin(target, candidatePos, exposedTo);
    threadData->worldBlockTargetShouldOperate = true;
    return ori;
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook5,
    ll::memory::HookPriority::Normal,
    NetherSpringFeature,
    &NetherSpringFeature::$place,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    if (pos.x % 16 >= 8 && pos.z % 16 >= 8) return origin(region, pos, random);
    auto  threadId   = std::this_thread::get_id();
    auto& controller = NetherDuplicatableController::getInstance();
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end()) {
            static_cast<NetherData*>(it->second->data.get())->springPosSet.emplace(pos);
            it->second->isEmpty = false;
        }
    }
    return origin(region, pos, random);
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook6,
    ll::memory::HookPriority::Normal,
    NetherFireFeature,
    &NetherFireFeature::$place,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    if (pos.x % 16 == 8 && pos.z % 16 == 8) return origin(region, pos, random);
    auto  threadId   = std::this_thread::get_id();
    auto& controller = NetherDuplicatableController::getInstance();
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end()) {
            static_cast<NetherData*>(it->second->data.get())->firePosMap.emplace(pos);
            it->second->isEmpty = false;
        }
    }
    return origin(region, pos, random);
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook7,
    ll::memory::HookPriority::Normal,
    GlowStoneFeature,
    &GlowStoneFeature::$place,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    if (pos.x % 16 == 8 && pos.z % 16 == 8) return origin(region, pos, random);
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = NetherDuplicatableController::getInstance();
    NetherThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end())
            threadData = static_cast<NetherThreadTemporaryData*>(it->second.get());
    }
    if (threadData) {
        if (ChunkPos(pos) != threadData->chunk->mPosition) {
            static_cast<NetherData*>(threadData->data.get())->glowStonePosMap.emplace(pos, threadData->temperaryInt++);
            threadData->isEmpty = false;
        } else if (region.getBlock(pos).isAir()) {
            auto& upBlockName = region.getBlock(BlockPos(pos.x, pos.y + 1, pos.z)).getTypeName();
            if (upBlockName == "minecraft:netherrack" || upBlockName == "minecraft:soul_sand"
                || upBlockName == "minecraft:blockstone") {
                static_cast<NetherData*>(threadData->data.get())
                    ->glowStonePosMap.emplace(pos, threadData->temperaryInt++);
                threadData->isEmpty = false;
            }
        }
    }
    return origin(region, pos, random);
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook8,
    ll::memory::HookPriority::Normal,
    MushroomFeature,
    &MushroomFeature::$place,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Random&         random
) {
    if (pos.x % 16 == 8 && pos.z % 16 == 8) return origin(region, pos, random);
    auto  threadId   = std::this_thread::get_id();
    auto& controller = NetherDuplicatableController::getInstance();
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end()) {
            static_cast<NetherData*>(it->second->data.get())
                ->mushroomPosMap.emplace(pos, this->mMushroomBlock.getTypeName() == "minecraft:red_mushroom");
            it->second->isEmpty = false;
        }
    }
    return origin(region, pos, random);
}

LL_TYPE_INSTANCE_HOOK(
    NetherDuplicatableController::NetherHook9,
    ll::memory::HookPriority::Normal,
    OreFeature,
    &OreFeature::$place,
    ::std::optional<::BlockPos>,
    ::IFeature::PlacementContext const& context
) {
    if (context.mPos->x % 16 < 8 && context.mPos->z % 16 < 8) return origin(context);
    auto                       threadId   = std::this_thread::get_id();
    auto&                      controller = NetherDuplicatableController::getInstance();
    NetherThreadTemporaryData* threadData = nullptr;
    {
        std::lock_guard lock(controller.decorationThreadIdsLock);
        auto            it = controller.decorationThreadIds.find(threadId);
        if (it != controller.decorationThreadIds.end())
            threadData = static_cast<NetherThreadTemporaryData*>(it->second.get());
    }
    if (!threadData) return origin(context);
    auto& blockName          = (*this->mReplaceRules)[0].mBlock->getBlockOrUnknownBlock().getTypeName();
    auto& duplicatableConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable;
    switch (blockName[10]) {
    case 'n':
        if (blockName == "minecraft:nether_gold_ore" && duplicatableConfig.netherGold.enable) {
            static_cast<NetherData*>(threadData->data.get())->netherGoldPosMap.emplace(context.mPos);
            threadData->isEmpty = false;
        }
        break;
    case 'q':
        if (blockName == "minecraft:quartz_ore" && duplicatableConfig.netherQuartz.enable) {
            static_cast<NetherData*>(threadData->data.get())->netherQuartzPosMap.emplace(context.mPos);
            threadData->isEmpty = false;
        }
        break;
    case 'm':
        if (blockName == "minecraft:magma" && duplicatableConfig.netherMagma.enable) {
            static_cast<NetherData*>(threadData->data.get())->netherMagmaPosMap.emplace(context.mPos);
            threadData->isEmpty = false;
        }
        break;
    case 'g':
        if (blockName == "minecraft:gravel" && duplicatableConfig.netherGravel.enable) {
            static_cast<NetherData*>(threadData->data.get())->netherGravelPosMap.emplace(context.mPos);
            threadData->isEmpty = false;
        }
        break;
    case 'b':
        if (blockName == "minecraft:blackstone" && duplicatableConfig.netherBlackstone.enable) {
            static_cast<NetherData*>(threadData->data.get())->blackstonePosMap.emplace(context.mPos);
            threadData->isEmpty = false;
        }
        break;
    case 's':
        if (blockName == "minecraft:soul_sand" && duplicatableConfig.netherSoulSand.enable) {
            static_cast<NetherData*>(threadData->data.get())->soulSandPosMap.emplace(context.mPos);
            threadData->isEmpty = false;
        }
        break;
    default:
        break;
    }
    return origin(context);
}

// ========== 单例 ==========

NetherDuplicatableController& NetherDuplicatableController::getInstance() {
    static NetherDuplicatableController instance;
    return instance;
}

NetherDuplicatableController::NetherDuplicatableController() {
    this->responsibleShowTypes = static_cast<uint>(DuplicatableManager::ShowType::Netherite)
                               | static_cast<uint>(DuplicatableManager::ShowType::NetherSpring)
                               | static_cast<uint>(DuplicatableManager::ShowType::NetherFire)
                               | static_cast<uint>(DuplicatableManager::ShowType::GlowStone)
                               | static_cast<uint>(DuplicatableManager::ShowType::Mushroom)
                               | static_cast<uint>(DuplicatableManager::ShowType::NetherGold)
                               | static_cast<uint>(DuplicatableManager::ShowType::NetherQuartz)
                               | static_cast<uint>(DuplicatableManager::ShowType::NetherMagma)
                               | static_cast<uint>(DuplicatableManager::ShowType::NetherGravel)
                               | static_cast<uint>(DuplicatableManager::ShowType::Blackstone)
                               | static_cast<uint>(DuplicatableManager::ShowType::SoulSand);
}

// ========== 绘制函数 ==========

bsci::GeometryGroup::GeoId
NetherDuplicatableController::drawNetherite(std::multimap<BlockPos, std::unordered_set<BlockPos>>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup   = CoralFans::getInstance().getGeometryGroup();
    auto& netheriteConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.netherite;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(5 * data.size());
    for (auto& [oriPos, poses] : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(oriPos, oriPos + BlockPos(1, 1, 1)), mce::Color(netheriteConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
            "translate.locate.duplicatable.netherite.oriPos"_tr(),
            mce::Color(netheriteConfig.textColor)
        ));
        for (auto& pos : poses) {
            geoIdList.emplace_back(
                geometryGroup->box(1, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(netheriteConfig.posColor))
            );
            geoIdList.emplace_back(geometryGroup->text(
                1,
                {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
                "translate.locate.duplicatable.netherite.pos"_tr(),
                mce::Color(netheriteConfig.textColor)
            ));
            geoIdList.emplace_back(geometryGroup->arrow(
                1,
                {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
                {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
                mce::Color(netheriteConfig.arrowColor),
                0.75f,
                0.2f
            ));
        }
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId NetherDuplicatableController::drawSpring(std::unordered_set<BlockPos>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup      = CoralFans::getInstance().getGeometryGroup();
    auto& netherSpringConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.netherSpring;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(3 * data.size());
    for (auto& pos : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(netherSpringConfig.posColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            "translate.locate.duplicatable.netherSpring.pos"_tr(),
            mce::Color(netherSpringConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            1,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(netherSpringConfig.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId NetherDuplicatableController::drawFire(std::unordered_set<BlockPos>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup    = CoralFans::getInstance().getGeometryGroup();
    auto& netherFireConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.netherFire;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(4 * data.size());
    for (auto& pos : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(netherFireConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            "translate.locate.duplicatable.netherFire.pos"_tr(),
            mce::Color(netherFireConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->box(
            1,
            AABB(pos - BlockPos(7, 3, 7), pos + BlockPos(8, 4, 8)),
            mce::Color(netherFireConfig.boundColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            1,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(netherFireConfig.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId NetherDuplicatableController::drawGlowStone(std::multimap<BlockPos, int>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup   = CoralFans::getInstance().getGeometryGroup();
    auto& glowStoneConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.glowStone;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(4 * data.size());
    for (auto& [pos, number] : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(glowStoneConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            "translate.locate.duplicatable.glowStone.pos"_tr(number),
            mce::Color(glowStoneConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->box(
            1,
            AABB(pos - BlockPos(7, 11, 7), pos + BlockPos(8, 1, 8)),
            mce::Color(glowStoneConfig.boundColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            1,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(glowStoneConfig.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId NetherDuplicatableController::drawMushroom(std::multimap<BlockPos, bool>& data) {
    using ll::i18n_literals::operator""_tr;
    auto& geometryGroup  = CoralFans::getInstance().getGeometryGroup();
    auto& mushroomConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable.mushroom;
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(4 * data.size());
    for (auto& [pos, isRed] : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(pos, pos + BlockPos(1, 1, 1)), mce::Color(mushroomConfig.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            isRed ? "translate.locate.duplicatable.mushroom.redPos"_tr()
                  : "translate.locate.duplicatable.mushroom.brownPos"_tr(),
            mce::Color(mushroomConfig.textColor)
        ));
        geoIdList.emplace_back(geometryGroup->box(
            1,
            AABB(pos - BlockPos(7, 3, 7), pos + BlockPos(8, 4, 8)),
            mce::Color(mushroomConfig.boundColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            1,
            {pos.x - 7.5f, pos.y + 0.5f, pos.z - 7.5f},
            {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f},
            mce::Color(mushroomConfig.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

bsci::GeometryGroup::GeoId NetherDuplicatableController::drawOre(
    std::unordered_set<BlockPos>&          data,
    config::Locate::DuplicatableOreStruct& config,
    std::string                            text
) {
    auto&                                   geometryGroup = CoralFans::getInstance().getGeometryGroup();
    std::vector<bsci::GeometryGroup::GeoId> geoIdList;
    geoIdList.reserve(3 * data.size());
    for (auto& oriPos : data) {
        geoIdList.emplace_back(
            geometryGroup->box(1, AABB(oriPos, oriPos + BlockPos(1, 1, 1)), mce::Color(config.originPosColor))
        );
        geoIdList.emplace_back(geometryGroup->text(
            1,
            {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
            text,
            mce::Color(config.originPosTextColor)
        ));
        geoIdList.emplace_back(geometryGroup->arrow(
            1,
            {oriPos.x + 0.5f, oriPos.y + 0.5f, oriPos.z + 0.5f},
            {oriPos.x + 8.5f, oriPos.y - 0.5f, oriPos.z + 8.5f},
            mce::Color(config.arrowColor)
        ));
    }
    return geometryGroup->merge(geoIdList);
}

// ========== removeAllGeoIds ==========

void NetherDuplicatableController::removeAllGeoIds(BsciChunkDataBase& data) {
    auto& netherData    = static_cast<NetherBsciChunkData&>(data);
    auto& geometryGroup = CoralFans::getInstance().getGeometryGroup();
    if (netherData.netheriteGeoId.value) {
        geometryGroup->remove(netherData.netheriteGeoId);
        netherData.netheriteGeoId.value = 0;
    }
    if (netherData.springGeoId.value) {
        geometryGroup->remove(netherData.springGeoId);
        netherData.springGeoId.value = 0;
    }
    if (netherData.fireGeoId.value) {
        geometryGroup->remove(netherData.fireGeoId);
        netherData.fireGeoId.value = 0;
    }
    if (netherData.glowStoneGeoId.value) {
        geometryGroup->remove(netherData.glowStoneGeoId);
        netherData.glowStoneGeoId.value = 0;
    }
    if (netherData.mushroomGeoId.value) {
        geometryGroup->remove(netherData.mushroomGeoId);
        netherData.mushroomGeoId.value = 0;
    }
    if (netherData.netherGoldGeoId.value) {
        geometryGroup->remove(netherData.netherGoldGeoId);
        netherData.netherGoldGeoId.value = 0;
    }
    if (netherData.netherQuartzGeoId.value) {
        geometryGroup->remove(netherData.netherQuartzGeoId);
        netherData.netherQuartzGeoId.value = 0;
    }
    if (netherData.netherMagmaGeoId.value) {
        geometryGroup->remove(netherData.netherMagmaGeoId);
        netherData.netherMagmaGeoId.value = 0;
    }
    if (netherData.netherGravelGeoId.value) {
        geometryGroup->remove(netherData.netherGravelGeoId);
        netherData.netherGravelGeoId.value = 0;
    }
    if (netherData.blackstoneGeoId.value) {
        geometryGroup->remove(netherData.blackstoneGeoId);
        netherData.blackstoneGeoId.value = 0;
    }
    if (netherData.soulSandGeoId.value) {
        geometryGroup->remove(netherData.soulSandGeoId);
        netherData.soulSandGeoId.value = 0;
    }
}

// ========== 接口实现 ==========

void NetherDuplicatableController::draw(BlockSource& region, ChunkPos chunkPos, uint showType) {
    using ll::i18n_literals::operator""_tr;

    std::lock_guard lock(this->dataMapLock);
    auto            it = this->dataMap.find(chunkPos);
    if (it == this->dataMap.end()) {
        std::lock_guard lock2(this->bsciChunkDataLock);
        this->tryRemoveChunkData(chunkPos);
        return;
    }

    auto& data = *static_cast<NetherData*>(it->second.get());

    // 检查是否有任何需要显示的数据
    bool hasData =
        (showType & static_cast<uint>(DuplicatableManager::ShowType::Netherite) && !data.netheritePosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherSpring) && !data.springPosSet.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherFire) && !data.firePosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::GlowStone) && !data.glowStonePosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::Mushroom) && !data.mushroomPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherGold) && !data.netherGoldPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherQuartz)
            && !data.netherQuartzPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherMagma) && !data.netherMagmaPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherGravel)
            && !data.netherGravelPosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::Blackstone) && !data.blackstonePosMap.empty())
        || (showType & static_cast<uint>(DuplicatableManager::ShowType::SoulSand) && !data.soulSandPosMap.empty());

    if (!hasData) return;

    if (!DuplicatableController::isChunkValid(region, chunkPos)) {
        this->dataMap.erase(it);
        std::lock_guard lock2(this->bsciChunkDataLock);
        this->tryRemoveChunkData(chunkPos);
        return;
    }

    auto& duplicatableConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable;

    std::lock_guard lock2(this->bsciChunkDataLock);
    auto [bsciIt, inserted] = this->bsciChunkData.try_emplace(chunkPos, std::make_unique<NetherBsciChunkData>());
    auto& bsciData          = *static_cast<NetherBsciChunkData*>(bsciIt->second.get());
    if (!bsciData.dataDrawed)
        DuplicatableController::drawChunkSavedInfo(region, chunkPos, bsciData, this->bsciChunkData);
    else if (data.reload) {
        this->removeAllGeoIds(bsciData);
        bsciData.dataDrawed = 0;
    }
    bsciData.runtimeRemoveTickCounter = 0;
    data.reload                       = false;

    if (showType & static_cast<uint>(DuplicatableManager::ShowType::Netherite) && !data.netheritePosMap.empty()
        && !bsciData.netheriteGeoId.value) {
        bsciData.netheriteGeoId  = this->drawNetherite(data.netheritePosMap);
        bsciData.dataDrawed     |= static_cast<uint>(DuplicatableManager::ShowType::Netherite);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherSpring) && !data.springPosSet.empty()
        && !bsciData.springGeoId.value) {
        bsciData.springGeoId  = this->drawSpring(data.springPosSet);
        bsciData.dataDrawed  |= static_cast<uint>(DuplicatableManager::ShowType::NetherSpring);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherFire) && !data.firePosMap.empty()
        && !bsciData.fireGeoId.value) {
        bsciData.fireGeoId   = this->drawFire(data.firePosMap);
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::NetherFire);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::GlowStone) && !data.glowStonePosMap.empty()
        && !bsciData.glowStoneGeoId.value) {
        bsciData.glowStoneGeoId  = this->drawGlowStone(data.glowStonePosMap);
        bsciData.dataDrawed     |= static_cast<uint>(DuplicatableManager::ShowType::GlowStone);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::Mushroom) && !data.mushroomPosMap.empty()
        && !bsciData.mushroomGeoId.value) {
        bsciData.mushroomGeoId  = this->drawMushroom(data.mushroomPosMap);
        bsciData.dataDrawed    |= static_cast<uint>(DuplicatableManager::ShowType::Mushroom);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherGold) && !data.netherGoldPosMap.empty()
        && !bsciData.netherGoldGeoId.value) {
        bsciData.netherGoldGeoId = drawOre(
            data.netherGoldPosMap,
            duplicatableConfig.netherGold,
            "translate.locate.duplicatable.netherGold.oriPos"_tr()
        );
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::NetherGold);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherQuartz) && !data.netherQuartzPosMap.empty()
        && !bsciData.netherQuartzGeoId.value) {
        bsciData.netherQuartzGeoId = drawOre(
            data.netherQuartzPosMap,
            duplicatableConfig.netherQuartz,
            "translate.locate.duplicatable.netherQuartz.oriPos"_tr()
        );
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::NetherQuartz);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherMagma) && !data.netherMagmaPosMap.empty()
        && !bsciData.netherMagmaGeoId.value) {
        bsciData.netherMagmaGeoId = drawOre(
            data.netherMagmaPosMap,
            duplicatableConfig.netherMagma,
            "translate.locate.duplicatable.netherMagma.oriPos"_tr()
        );
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::NetherMagma);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::NetherGravel) && !data.netherGravelPosMap.empty()
        && !bsciData.netherGravelGeoId.value) {
        bsciData.netherGravelGeoId = drawOre(
            data.netherGravelPosMap,
            duplicatableConfig.netherGravel,
            "translate.locate.duplicatable.netherGravel.oriPos"_tr()
        );
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::NetherGravel);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::Blackstone) && !data.blackstonePosMap.empty()
        && !bsciData.blackstoneGeoId.value) {
        bsciData.blackstoneGeoId = drawOre(
            data.blackstonePosMap,
            duplicatableConfig.netherBlackstone,
            "translate.locate.duplicatable.netherBlackstone.oriPos"_tr()
        );
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::Blackstone);
    }
    if (showType & static_cast<uint>(DuplicatableManager::ShowType::SoulSand) && !data.soulSandPosMap.empty()
        && !bsciData.soulSandGeoId.value) {
        bsciData.soulSandGeoId = drawOre(
            data.soulSandPosMap,
            duplicatableConfig.netherSoulSand,
            "translate.locate.duplicatable.netherSoulSand.oriPos"_tr()
        );
        bsciData.dataDrawed |= static_cast<uint>(DuplicatableManager::ShowType::SoulSand);
    }
}

bsci::GeometryGroup::GeoId* NetherDuplicatableController::getGeoIdByShowType(BsciChunkDataBase& data, uint showType) {
    auto& netherData = static_cast<NetherBsciChunkData&>(data);
    switch (static_cast<DuplicatableManager::ShowType>(showType)) {
    case DuplicatableManager::ShowType::Netherite:
        return &netherData.netheriteGeoId;
    case DuplicatableManager::ShowType::NetherSpring:
        return &netherData.springGeoId;
    case DuplicatableManager::ShowType::NetherFire:
        return &netherData.fireGeoId;
    case DuplicatableManager::ShowType::GlowStone:
        return &netherData.glowStoneGeoId;
    case DuplicatableManager::ShowType::Mushroom:
        return &netherData.mushroomGeoId;
    case DuplicatableManager::ShowType::NetherGold:
        return &netherData.netherGoldGeoId;
    case DuplicatableManager::ShowType::NetherQuartz:
        return &netherData.netherQuartzGeoId;
    case DuplicatableManager::ShowType::NetherMagma:
        return &netherData.netherMagmaGeoId;
    case DuplicatableManager::ShowType::NetherGravel:
        return &netherData.netherGravelGeoId;
    case DuplicatableManager::ShowType::Blackstone:
        return &netherData.blackstoneGeoId;
    case DuplicatableManager::ShowType::SoulSand:
        return &netherData.soulSandGeoId;
    default:
        return nullptr;
    }
}

bool NetherDuplicatableController::isResponsibleFor(uint showType) const {
    return showType & this->responsibleShowTypes;
}

void NetherDuplicatableController::hook(bool enable) {
    if (enable) {
        auto& duplicatableConfig = CoralFans::getInstance().getConfig().functions.locate.duplicatable;
        bool  shouldHook         = false;
        if (duplicatableConfig.netherite.enable) {
            NetherHook2::hook();
            NetherHook3::hook();
            NetherHook4::hook();
            shouldHook = true;
        }
        if (duplicatableConfig.netherSpring.enable) {
            NetherHook5::hook();
            shouldHook = true;
        }
        if (duplicatableConfig.netherFire.enable) {
            NetherHook6::hook();
            shouldHook = true;
        }
        if (duplicatableConfig.glowStone.enable) {
            NetherHook7::hook();
            shouldHook = true;
        }
        if (duplicatableConfig.mushroom.enable) {
            NetherHook8::hook();
            shouldHook = true;
        }
        if (duplicatableConfig.netherGold.enable || duplicatableConfig.netherQuartz.enable
            || duplicatableConfig.netherMagma.enable || duplicatableConfig.netherGravel.enable
            || duplicatableConfig.netherBlackstone.enable || duplicatableConfig.netherSoulSand.enable) {
            NetherHook9::hook();
            shouldHook = true;
        }
        if (shouldHook) NetherHook1::hook();
    } else {
        NetherHook1::unhook();
        NetherHook2::unhook();
        NetherHook3::unhook();
        NetherHook4::unhook();
        NetherHook5::unhook();
        NetherHook6::unhook();
        NetherHook7::unhook();
        NetherHook8::unhook();
        NetherHook9::unhook();
    }
}

} // namespace coral_fans::functions::locate
