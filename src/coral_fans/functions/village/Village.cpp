#include "coral_fans/functions/village/Village.h"
#include "Village.h"
#include "coral_fans/CoralFans.h"


#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Color.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/platform/UUID.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ai/village/POIInstance.h"
#include "mc/world/actor/ai/village/Village.h"
#include "mc/world/actor/ai/village/VillageManager.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Tick.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/phys/AABB.h"


#ifdef LL_PLAT_C
#include "mc/server/ServerInstance.h"
#include <thread>
#endif

#include <cstddef>
#include <math.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace coral_fans::functions {
float getApproximateRadius(AABB& bound) {
    float x = bound.max.x - bound.min.x;
    float z = bound.max.z - bound.min.z;
    if (fabs(x - z) >= 0.00000011920929f) return (x + z) * 0.3f;
    else return 1.414213562373f * x;
}

CFTickingVillageData::CFTickingVillageData(const Village& village, Tick& tick) {
    if (auto level = ll::service::getLevel()) [[likely]] {
        if (auto dim = level->getDimension(village.mDimension.getDimensionId()).lock()) [[likely]] {
            mVillagePtr = dim->mVillageManager->getVillageByID(village.mUniqueID);
        }
    }
    mDimId      = village.mDimension.getDimensionId();
    mLastTick   = tick;
    mBounds     = village.mBounds;
    mRaidBounds = village.mStaticRaidBounds;
}

void CFTickingVillageData::showBounds() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(mBoundsGeoId);
    mBoundsGeoId =
        mod.getGeometryGroup()->box(mDimId, mBounds, mce::Color(mod.getConfig().functions.village.boundsColor));
}

void CFTickingVillageData::showRaidBounds() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(mRaidBoundsGeoId);
    mRaidBoundsGeoId =
        mod.getGeometryGroup()->box(mDimId, mRaidBounds, mce::Color(mod.getConfig().functions.village.raidBoundsColor));
}

void CFTickingVillageData::showIronSpawn() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(mIronSpawnGeoId);
    mIronSpawnGeoId = mod.getGeometryGroup()->box(
        mDimId,
        {
            mBounds.center() - Vec3{8, 6, 8},
            mBounds.center() + Vec3{9, 7, 9}
    },
        mce::Color(mod.getConfig().functions.village.ironSpawnBoundsColor)
    );
}

void CFTickingVillageData::showCenter() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(mCenterGeoId);
    mCenterGeoId = mod.getGeometryGroup()->box(
        mDimId,
        {
            mBounds.center(),
            mBounds.center() + Vec3{1, 1, 1}
    },
        mce::Color(mod.getConfig().functions.village.centerColor)
    );
}

void CFTickingVillageData::showPoiQuery() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(mPoiQueryGeoId);
    mPoiQueryGeoId = mod.getGeometryGroup()->box(
        mDimId,
        {
            mBounds.min - Vec3{64, 64, 64},
            mBounds.max + Vec3{64, 64, 64}
    },
        mce::Color(mod.getConfig().functions.village.poiBoundsColor)
    );
}

void CFTickingVillageData::showBind() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(mBindGeoId);
    if (auto level = ll::service::getLevel()) {
        if (auto villagePtr = mVillagePtr.lock()) [[likely]] {
            std::vector<bsci::GeometryGroup::GeoId> bindIds;
            const static mce::Color                 colors[3] = {
                mce::Color(mod.getConfig().functions.village.bedBindColor),
                mce::Color(mod.getConfig().functions.village.ringBindColor),
                mce::Color(mod.getConfig().functions.village.workBindColor)
            };
            bindIds.reserve(3 * villagePtr->mClaimedPOIs->size());
            for (auto& [actorUniqueId, poiArray] : *villagePtr->mClaimedPOIs) {
                auto villager = level->fetchEntity(actorUniqueId, false);
                if (!villager) continue;
                for (int i = 0; i < 3; i++) {
                    if (auto poi = poiArray[i].lock()) {
                        bindIds.emplace_back(mod.getGeometryGroup()->line(
                            villager->getDimensionId(),
                            villager->getHeadPos(),
                            poi->mPosition->center(),
                            colors[i]
                        ));
                    }
                }
            }
            mBindGeoId = mod.getGeometryGroup()->merge(bindIds);
        }
    }
}

int CFVillageManager::getVillageId(std::weak_ptr<Village> villagePtr) {
    if (auto village = villagePtr.lock()) [[likely]] {
        auto size = mVillageList.size();
        for (size_t i = 0; i < size; i++) {
            if (mVillageList[i]) {
                if (auto lockedVillage = mVillageList[i].value().lock()) {
                    if (lockedVillage == village) return static_cast<int>(i);
                } else mVillageList[i] = std::nullopt;
            }
        }
    }
    return -1;
}

void CFVillageManager::addVillage(std::weak_ptr<Village> villagePtr) { mVillageList.emplace_back(villagePtr); }

void CFVillageManager::handleVillageTick(const Village& village, Tick& tick) {
    auto [it, isInserted] = this->mTickingList.try_emplace(village.mUniqueID);
    if (isInserted) {
        it->second = std::make_unique<CFTickingVillageData>(village, tick);
        if (mShowBounds) it->second->showBounds();
        if (mShowRaidBounds) it->second->showRaidBounds();
        if (mShowIronSpawn) it->second->showIronSpawn();
        if (mShowCenter) it->second->showCenter();
        if (mShowPoiQuery) it->second->showPoiQuery();
        if (mShowBind) it->second->showBind();
    } else {
        it->second->mLastTick = tick;
    }
}

void CFVillageManager::tick(const Tick& currentTick) {
    auto&      mod      = CoralFans::getInstance();
    auto&      geoGroup = mod.getGeometryGroup();
    static int gt       = 0;
    std::erase_if(
        mTickingList,
        [this, &currentTick, &geoGroup](std::pair<const mce::UUID, std::unique_ptr<CFTickingVillageData>>& item) {
            auto& [uuid, villageData] = item;
            if (currentTick.tickID - villageData->mLastTick.tickID > 5) {
                if (mShowBounds) geoGroup->remove(villageData->mBoundsGeoId);
                if (mShowRaidBounds) geoGroup->remove(villageData->mRaidBoundsGeoId);
                if (mShowIronSpawn) geoGroup->remove(villageData->mIronSpawnGeoId);
                if (mShowCenter) geoGroup->remove(villageData->mCenterGeoId);
                if (mShowPoiQuery) geoGroup->remove(villageData->mPoiQueryGeoId);
                if (mShowBind) geoGroup->remove(villageData->mBindGeoId);
                return true;
            }
            auto village = villageData->mVillagePtr.lock();
            if (!village) [[unlikely]] {
                if (mShowBounds) geoGroup->remove(villageData->mBoundsGeoId);
                if (mShowRaidBounds) geoGroup->remove(villageData->mRaidBoundsGeoId);
                if (mShowIronSpawn) geoGroup->remove(villageData->mIronSpawnGeoId);
                if (mShowCenter) geoGroup->remove(villageData->mCenterGeoId);
                if (mShowPoiQuery) geoGroup->remove(villageData->mPoiQueryGeoId);
                if (mShowBind) geoGroup->remove(villageData->mBindGeoId);
                return true;
            }
            if (villageData->mBounds != village->mBounds) {
                villageData->mBounds = village->mBounds;
                if (mShowBounds) {
                    geoGroup->remove(villageData->mBoundsGeoId);
                    villageData->showBounds();
                }
                if (mShowIronSpawn) {
                    geoGroup->remove(villageData->mIronSpawnGeoId);
                    villageData->showIronSpawn();
                }
                if (mShowCenter) {
                    geoGroup->remove(villageData->mCenterGeoId);
                    villageData->showCenter();
                }
                if (mShowPoiQuery) {
                    geoGroup->remove(villageData->mPoiQueryGeoId);
                    villageData->showPoiQuery();
                }
            }
            if (villageData->mRaidBounds != village->mStaticRaidBounds) {
                villageData->mRaidBounds = village->mStaticRaidBounds;
                if (this->mShowRaidBounds) {
                    geoGroup->remove(villageData->mRaidBoundsGeoId);
                    villageData->showRaidBounds();
                }
            }
            if (!gt && this->mShowBind) {
                geoGroup->remove(villageData->mBindGeoId);
                villageData->showBind();
            }
            return false;
        }
    );
    const static int interval = std::max(1, mod.getConfig().functions.village.drawInterval);
    gt                        = (gt + 1) % interval;
}

void CFVillageManager::setShowBounds(bool show) {
    mShowBounds = show;
    if (show) {
        for (auto& [uuid, villageData] : this->mTickingList) {
            villageData->showBounds();
        }
    } else {
        for (auto& [uuid, villageData] : this->mTickingList) {
            CoralFans::getInstance().getGeometryGroup()->remove(villageData->mBoundsGeoId);
        }
    }
}

void CFVillageManager::setShowRaidBounds(bool show) {
    this->mShowRaidBounds = show;
    if (show) {
        for (auto& [uuid, villageData] : this->mTickingList) {
            villageData->showRaidBounds();
        }
    } else {
        for (auto& [uuid, villageData] : this->mTickingList) {
            CoralFans::getInstance().getGeometryGroup()->remove(villageData->mRaidBoundsGeoId);
        }
    }
}

void CFVillageManager::setShowIronSpawn(bool show) {
    this->mShowIronSpawn = show;
    if (show) {
        for (auto& [uuid, villageData] : this->mTickingList) {
            villageData->showIronSpawn();
        }
    } else {
        for (auto& [uuid, villageData] : this->mTickingList) {
            CoralFans::getInstance().getGeometryGroup()->remove(villageData->mIronSpawnGeoId);
        }
    }
}

void CFVillageManager::setShowCenter(bool show) {
    this->mShowCenter = show;
    if (show) {
        for (auto& [uuid, villageData] : this->mTickingList) {
            villageData->showCenter();
        }
    } else {
        for (auto& [uuid, villageData] : this->mTickingList) {
            CoralFans::getInstance().getGeometryGroup()->remove(villageData->mCenterGeoId);
        }
    }
}

void CFVillageManager::setShowPoiQuery(bool show) {
    this->mShowPoiQuery = show;
    if (show) {
        for (auto& [uuid, villageData] : this->mTickingList) {
            villageData->showPoiQuery();
        }
    } else {
        for (auto& [uuid, villageData] : this->mTickingList) {
            CoralFans::getInstance().getGeometryGroup()->remove(villageData->mPoiQueryGeoId);
        }
    }
}

void CFVillageManager::setShowBind(bool show) {
    this->mShowBind = show;
    if (show) {
        for (auto& [uuid, villageData] : this->mTickingList) {
            villageData->showBind();
        }
    } else {
        for (auto& [uuid, villageData] : this->mTickingList) {
            CoralFans::getInstance().getGeometryGroup()->remove(villageData->mBindGeoId);
        }
    }
}

bool CFVillageManager::getShowBounds() { return this->mShowBounds; }

bool CFVillageManager::getShowRaidBounds() { return this->mShowRaidBounds; }

bool CFVillageManager::getShowIronSpawn() { return this->mShowIronSpawn; }

bool CFVillageManager::getShowCenter() { return this->mShowCenter; }

bool CFVillageManager::getShowPoiQuery() { return this->mShowPoiQuery; }

bool CFVillageManager::getShowBind() { return this->mShowBind; }

int CFVillageManager::getBedPOICount(std::shared_ptr<Village> villagePtr) {
    if (!villagePtr) return 0;

    int count = 0;
    for (auto& [villagerId, poiList] : *villagePtr->mClaimedPOIs) {
        for (auto& poi : poiList) {
            auto poiPtr = poi.lock();
            if (poiPtr && poiPtr->mType == POIType::Bed) {
                count++;
            }
        }
    }
    for (auto& poiStack : *villagePtr->mUnclaimedPOIStacks) {
        for (auto& poi : poiStack) {
            auto poiPtr = poi.lock();
            if (poiPtr && poiPtr->mType == POIType::Bed) {
                count++;
            }
        }
    }
    return count;
}

std::vector<std::string> CFVillageManager::listVillages() {
    using ll::i18n_literals::operator""_tr;
    size_t                   size = mVillageList.size();
    std::vector<std::string> retstrs;
    for (size_t i = 0; i < size; i++) {
        std::string retstr = "";
        if (!mVillageList[i]) retstr += "translate.village.villageHasBeenRemoved"_tr(i);
        else {
            auto villagePtr = mVillageList[i].value().lock();
            if (!villagePtr) {
                retstr          += "translate.village.villageHasBeenRemoved"_tr(i);
                mVillageList[i]  = std::nullopt;
                continue;
            }
            std::string dimensionText = "";
            switch (villagePtr->mDimension.getDimensionId()) {
            case 0:
                dimensionText = "translate.dimension.overworld"_tr();
                break;
            case 1:
                dimensionText = "translate.dimension.nether"_tr();
                break;
            case 2:
                dimensionText = "translate.dimension.the_end"_tr();
                break;
            default:
                dimensionText = "translate.dimension.unknown"_tr();
            }
            retstr += "translate.village.villageInfo"_tr(
                i,
                villagePtr->mBounds->center().toString(),
                dimensionText,
                getApproximateRadius(*villagePtr->mBounds),
                (*villagePtr->mDwellers)[0].size(), // Villager
                (*villagePtr->mDwellers)[1].size(), // IronGolem
                CFVillageManager::getBedPOICount(villagePtr),
                villagePtr->mBounds->min.toString(),
                villagePtr->mBounds->max.toString()
            );
        }
        retstrs.emplace_back(std::move(retstr));
    }
    return retstrs;
}

std::string CFVillageManager::listTickingVillages() {
    using ll::i18n_literals::operator""_tr;
    std::string retstr = "";
    for (auto& [uuid, villageData] : this->mTickingList) {
        if (auto villagePtr = villageData->mVillagePtr.lock()) {
            size_t      id            = this->getVillageId(villagePtr);
            std::string dimensionText = "";
            switch (villagePtr->mDimension.getDimensionId()) {
            case 0:
                dimensionText = "translate.dimension.overworld"_tr();
                break;
            case 1:
                dimensionText = "translate.dimension.nether"_tr();
                break;
            case 2:
                dimensionText = "translate.dimension.the_end"_tr();
                break;
            default:
                dimensionText = "translate.dimension.unknown"_tr();
            }
            retstr += "translate.village.villageInfo"_tr(
                id,
                villagePtr->mBounds->center().toString(),
                dimensionText,
                getApproximateRadius(*villagePtr->mBounds),
                (*villagePtr->mDwellers)[0].size(), // Villager
                (*villagePtr->mDwellers)[1].size(), // IronGolem
                CFVillageManager::getBedPOICount(villagePtr),
                villagePtr->mBounds->min.toString(),
                villagePtr->mBounds->max.toString()
            );
        }
    }
    return retstr;
}

std::string getPoiInfo(std::array<::std::weak_ptr<::POIInstance>, 3>& poiList) {
    using ll::i18n_literals::operator""_tr;
    std::string retstr  = "";
    retstr             += "translate.village.villagerPOIType1"_tr();
    auto poi1           = poiList[0].lock();
    if (poi1)
        retstr += "translate.village.villagerPOIInfo"_tr(
            *poi1->mPosition,
            poi1->mOwnerCount,
            poi1->mOwnerCapacity,
            poi1->mRadius,
            poi1->mWeight
        );
    else retstr += " §7(x)§r\n";
    retstr    += "translate.village.villagerPOIType2"_tr();
    auto poi2  = poiList[1].lock();
    if (poi2)
        retstr += "translate.village.villagerPOIInfo"_tr(
            *poi2->mPosition,
            poi2->mOwnerCount,
            poi2->mOwnerCapacity,
            poi2->mRadius,
            poi2->mWeight
        );
    else retstr += " §7(x)§r\n";
    retstr    += "translate.village.villagerPOIType3"_tr();
    auto poi3  = poiList[2].lock();
    if (poi3)
        retstr += "translate.village.villagerPOIInfo"_tr(
            *poi3->mPosition,
            poi3->mOwnerCount,
            poi3->mOwnerCapacity,
            poi3->mRadius,
            poi3->mWeight
        );
    else retstr += " §7(x)§r\n";
    return retstr;
}

std::pair<std::string, bool> CFVillageManager::getVillageInfo(int id) {
    using ll::i18n_literals::operator""_tr;
    if (id >= static_cast<int>(mVillageList.size())) return {"translate.village.notExist"_tr(), false};
    if (!mVillageList[id]) return {"translate.village.villageHasBeenRemoved"_tr(id), false};
    auto village = mVillageList[id].value().lock();
    if (!village) {
        mVillageList[id] = std::nullopt;
        return {"translate.village.villageHasBeenRemoved"_tr(id), false};
    }

    std::string dimensionText = "";
    switch (village->mDimension.getDimensionId()) {
    case 0:
        dimensionText = "translate.dimension.overworld"_tr();
        break;
    case 1:
        dimensionText = "translate.dimension.nether"_tr();
        break;
    case 2:
        dimensionText = "translate.dimension.the_end"_tr();
        break;
    default:
        dimensionText = "translate.dimension.unknown"_tr();
    }

    std::string retstr = "translate.village.info"_tr(
        id,
        village->mUniqueID->asString(),
        village->mBounds->center().toString(),
        dimensionText,
        village->mBounds->min.toString(),
        village->mBounds->max.toString(),
        getApproximateRadius(*village->mBounds),
        (*village->mDwellers)[0].size(),
        (*village->mDwellers)[1].size(),
        (*village->mDwellers)[2].size(),
        (*village->mDwellers)[3].size()
    );
    if (auto level = ll::service::getLevel()) {
        int i = 0;
        for (auto& [auid, poiList] : *village->mClaimedPOIs) {
            i++;
            retstr += "translate.village.villagerInfo"_tr(i, level->fetchEntity(auid, true)->getFeetPos().toString());
            retstr += getPoiInfo(poiList);
        }
    }
    return {retstr, true};
}

std::pair<std::string, bool> CFVillageManager::getVillagerInfo(ActorUniqueID auid) {
    using ll::i18n_literals::operator""_tr;
    for (auto& [uuid, villageData] : this->mTickingList) {
        auto village = villageData->mVillagePtr.lock();
        if (!village) continue;
        auto it = village->mClaimedPOIs->find(auid);
        if (it != village->mClaimedPOIs->end()) {
            std::string retstr = "";
            if (auto level = ll::service::getLevel()) {
                retstr = "translate.village.villagerInfo2"_tr(
                    this->getVillageId(villageData->mVillagePtr),
                    level->fetchEntity(auid, false)->getFeetPos().toString()
                );
                retstr += getPoiInfo(it->second);
            }
            return {retstr, true};
        }
    }
    return {"translate.village.cannotgetvillager"_tr(), false};
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageAddHook,
    ll::memory::HookPriority::Normal,
    Dimension,
    &Dimension ::$init,
    void,
    ::br::worldgen::StructureSetRegistry const& structureSetRegistry
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(structureSetRegistry);
#endif
    origin(structureSetRegistry);
    for (auto& [uuid, village] : *mVillageManager->mVillages) {
        CFVillageManager::getInstance().addVillage(village);
    }
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageAddHook2,
    ll::memory::HookPriority::Normal,
    VillageManager,
    &VillageManager ::_tryAssignPOIOrCreateVillage,
    void,
    ::std::shared_ptr<::POIInstance>&& pi
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(std::move(pi));
#endif
    auto& manager              = CFVillageManager::getInstance();
    manager.newVillageId       = std::nullopt;
    manager.mayCreatingVillage = true;
    origin(std::move(pi));
    manager.mayCreatingVillage = false;
    if (manager.newVillageId) {
        auto newVillage = getVillageByID(*manager.newVillageId);
        if (!newVillage.expired()) {
            manager.addVillage(newVillage);
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageAddHook3,
    ll::memory::HookPriority::Normal,
    Village,
    &Village::$ctor,
    void*,
    ::Dimension&      dimension,
    ::mce::UUID       id,
    ::BlockPos const& _origin
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(dimension, id, _origin);
#endif
    auto  ori     = origin(dimension, id, _origin);
    auto& manager = CFVillageManager::getInstance();
    if (manager.mayCreatingVillage) manager.newVillageId = id;
    return ori;
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageTickHook,
    ll::memory::HookPriority::Normal,
    Village,
    &Village::tick,
    void,
    Tick         tick,
    BlockSource& region
) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(tick, region);
#endif
    CFVillageManager::getInstance().handleVillageTick(*this, tick);
    origin(tick, region);
}

void CFVillageManager::hookVillage(bool hook) {
    if (hook) {
        CoralFansVillageAddHook::hook();
        CoralFansVillageAddHook2::hook();
        CoralFansVillageAddHook3::hook();
        CoralFansVillageTickHook::hook();
    } else {
        CoralFansVillageAddHook::unhook();
        CoralFansVillageAddHook2::unhook();
        CoralFansVillageAddHook3::unhook();
        CoralFansVillageTickHook::unhook();
    }
}

void CFVillageManager::clear() {
    setShowBounds(false);
    setShowRaidBounds(false);
    setShowIronSpawn(false);
    setShowCenter(false);
    setShowPoiQuery(false);
    setShowBind(false);
    mVillageList.clear();
    mTickingList.clear();
}
} // namespace coral_fans::functions