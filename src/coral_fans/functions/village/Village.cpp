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
#include "mc/server/ServerInstance.h"
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
#include <thread>

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

CFTickingVillageData::CFTickingVillageData(Village* villagePtr, Tick& tick) {
    this->mVillagePtr    = villagePtr;
    this->mLastTick      = tick;
    this->mOldBounds     = villagePtr->mBounds;
    this->mOldRaidBounds = villagePtr->mStaticRaidBounds;
}

void CFTickingVillageData::showBounds() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(this->mBoundsGeoId);
    this->mBoundsGeoId = mod.getGeometryGroup()->box(
        this->mVillagePtr->mDimension.getDimensionId(),
        this->mVillagePtr->mBounds,
        mce::Color(mod.getConfig().functions.village.boundsColor)
    );
}

void CFTickingVillageData::showRaidBounds() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(this->mRaidBoundsGeoId);
    this->mRaidBoundsGeoId = mod.getGeometryGroup()->box(
        this->mVillagePtr->mDimension.getDimensionId(),
        this->mVillagePtr->mStaticRaidBounds,
        mce::Color(mod.getConfig().functions.village.raidBoundsColor)
    );
}

void CFTickingVillageData::showIronSpawn() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(this->mIronSpawnGeoId);
    this->mIronSpawnGeoId = mod.getGeometryGroup()->box(
        this->mVillagePtr->mDimension.getDimensionId(),
        {
            this->mVillagePtr->mBounds->center() - Vec3{8, 6, 8},
            this->mVillagePtr->mBounds->center() + Vec3{9, 7, 9}
    },
        mce::Color(mod.getConfig().functions.village.ironSpawnBoundsColor)
    );
}

void CFTickingVillageData::showCenter() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(this->mCenterGeoId);
    this->mCenterGeoId = mod.getGeometryGroup()->box(
        this->mVillagePtr->mDimension.getDimensionId(),
        {
            this->mVillagePtr->mBounds->center(),
            this->mVillagePtr->mBounds->center() + Vec3{1, 1, 1}
    },
        mce::Color(mod.getConfig().functions.village.centerColor)
    );
}

void CFTickingVillageData::showPoiQuery() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(this->mPoiQueryGeoId);
    this->mPoiQueryGeoId = mod.getGeometryGroup()->box(
        this->mVillagePtr->mDimension.getDimensionId(),
        {
            this->mVillagePtr->mBounds->min - Vec3{64, 64, 64},
            this->mVillagePtr->mBounds->max + Vec3{64, 64, 64}
    },
        mce::Color(mod.getConfig().functions.village.poiBoundsColor)
    );
}

void CFTickingVillageData::showBind() {
    auto& mod = CoralFans::getInstance();
    mod.getGeometryGroup()->remove(this->mBindGeoId);
    if (auto level = ll::service::getLevel()) {
        std::vector<bsci::GeometryGroup::GeoId> bindIds;
        const static mce::Color                 colors[3] = {
            mce::Color(mod.getConfig().functions.village.bedBindColor),
            mce::Color(mod.getConfig().functions.village.ringBindColor),
            mce::Color(mod.getConfig().functions.village.workBindColor)
        };
        bindIds.reserve(3 * this->mVillagePtr->mClaimedPOIs->size());
        for (auto& [actorUniqueId, poiArray] : *this->mVillagePtr->mClaimedPOIs) {
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
        this->mBindGeoId = mod.getGeometryGroup()->merge(bindIds);
    }
}

int CFVillageManager::getVillageId(Village* villagePtr) {
    int size = static_cast<int>(this->mVillageList.size());
    for (int i = 0; i < size; i++) {
        if (this->mVillageList[i] == villagePtr) return i;
    }
    return -1;
}

void CFVillageManager::addVillage(Village* villagePtr) { this->mVillageList.emplace_back(villagePtr); }

void CFVillageManager::handleVillageTick(Village* villagePtr, Tick& tick) {
    auto [it, isInserted] = this->mTickingList.try_emplace(villagePtr->mUniqueID);
    if (isInserted) {
        it->second = std::make_unique<CFTickingVillageData>(villagePtr, tick);
        if (this->mShowBounds) it->second->showBounds();
        if (this->mShowRaidBounds) it->second->showRaidBounds();
        if (this->mShowIronSpawn) it->second->showIronSpawn();
        if (this->mShowCenter) it->second->showCenter();
        if (this->mShowPoiQuery) it->second->showPoiQuery();
        if (this->mShowBind) it->second->showBind();
    } else {
        it->second->mLastTick = tick;
    }
}

void CFVillageManager::removeVillage(Village* villagePtr) {
    auto it = this->mTickingList.find(villagePtr->mUniqueID);
    if (it != this->mTickingList.end()) {
        auto& geoGroup = CoralFans::getInstance().getGeometryGroup();
        if (this->mShowBounds) geoGroup->remove(it->second->mBoundsGeoId);
        if (this->mShowRaidBounds) geoGroup->remove(it->second->mRaidBoundsGeoId);
        if (this->mShowIronSpawn) geoGroup->remove(it->second->mIronSpawnGeoId);
        if (this->mShowCenter) geoGroup->remove(it->second->mCenterGeoId);
        if (this->mShowPoiQuery) geoGroup->remove(it->second->mPoiQueryGeoId);
        if (this->mShowBind) geoGroup->remove(it->second->mBindGeoId);
        this->mTickingList.erase(it);
    }
    auto size = this->mVillageList.size();
    for (size_t i = 0; i < size; i++) {
        if (mVillageList[i] == villagePtr) {
            mVillageList[i] = nullptr;
            break;
        }
    }
}

void CFVillageManager::tick(const Tick& currentTick) {
    auto&      mod      = CoralFans::getInstance();
    auto&      geoGroup = mod.getGeometryGroup();
    static int gt       = 8;
    std::erase_if(
        this->mTickingList,
        [this, &currentTick, &geoGroup](std::pair<const mce::UUID, std::unique_ptr<CFTickingVillageData>>& item) {
            auto& [uuid, villageData] = item;
            if (currentTick.tickID - villageData->mLastTick.tickID > 5) {
                if (this->mShowBounds) geoGroup->remove(villageData->mBoundsGeoId);
                if (this->mShowRaidBounds) geoGroup->remove(villageData->mRaidBoundsGeoId);
                if (this->mShowIronSpawn) geoGroup->remove(villageData->mIronSpawnGeoId);
                if (this->mShowCenter) geoGroup->remove(villageData->mCenterGeoId);
                if (this->mShowPoiQuery) geoGroup->remove(villageData->mPoiQueryGeoId);
                if (this->mShowBind) geoGroup->remove(villageData->mBindGeoId);
                return true;
            }
            if (villageData->mOldBounds != villageData->mVillagePtr->mBounds) {
                villageData->mOldBounds = villageData->mVillagePtr->mBounds;
                if (this->mShowBounds) {
                    geoGroup->remove(villageData->mBoundsGeoId);
                    villageData->showBounds();
                }
                if (this->mShowIronSpawn) {
                    geoGroup->remove(villageData->mIronSpawnGeoId);
                    villageData->showIronSpawn();
                }
                if (this->mShowCenter) {
                    geoGroup->remove(villageData->mCenterGeoId);
                    villageData->showCenter();
                }
                if (this->mShowPoiQuery) {
                    geoGroup->remove(villageData->mPoiQueryGeoId);
                    villageData->showPoiQuery();
                }
            }
            if (villageData->mOldRaidBounds != villageData->mVillagePtr->mStaticRaidBounds) {
                villageData->mOldRaidBounds = villageData->mVillagePtr->mStaticRaidBounds;
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
    this->mShowBounds = show;
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

std::vector<std::string> CFVillageManager::listVillages() {
    using ll::i18n_literals::operator""_tr;
    size_t                   size = this->mVillageList.size();
    std::vector<std::string> retstrs;
    for (size_t i = 0; i < size; i++) {
        std::string retstr = "";
        if (!this->mVillageList[i]) retstr += "translate.village.villageHasBeenRemoved"_tr(i);
        else {
            auto&       villagePtr    = this->mVillageList[i];
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
                villagePtr->getBedPOICount(),
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
        auto        villagePtr    = villageData->mVillagePtr;
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
            villagePtr->getBedPOICount(),
            villagePtr->mBounds->min.toString(),
            villagePtr->mBounds->max.toString()
        );
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
    if (id >= static_cast<int>(this->mVillageList.size())) return {"translate.village.notExist"_tr(), false};
    auto village = this->mVillageList[id];
    if (!village) return {"translate.village.villageHasBeenRemoved"_tr(id), false};

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
        auto it = villageData->mVillagePtr->mClaimedPOIs->find(auid);
        if (it != villageData->mVillagePtr->mClaimedPOIs->end()) {
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

// hook village tick

LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageAddHook,
    ll::memory::HookPriority::Normal,
    Village,
    &Village::$ctor,
    void*,
    ::Dimension&      dimension,
    ::mce::UUID       id,
    ::BlockPos const& _origin
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(dimension, id, _origin);
#endif
    auto ori = origin(dimension, id, _origin);
    CFVillageManager::getInstance().addVillage(static_cast<Village*>(ori));
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
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(tick, region);
#endif
    CFVillageManager::getInstance().handleVillageTick(this, tick);
    origin(tick, region);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageRemoveHook,
    ll::memory::HookPriority::Normal,
    VillageManager,
    &VillageManager::_removeVillage,
    void,
    ::Village& village
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(village);
#endif
    CFVillageManager::getInstance().removeVillage(&village);
    origin(village);
}

void CFVillageManager::hookVillage(bool hook) {
    if (hook) {
        CoralFansVillageAddHook::hook();
        CoralFansVillageTickHook::hook();
        CoralFansVillageRemoveHook::hook();
    } else {
        CoralFansVillageAddHook::unhook();
        CoralFansVillageTickHook::unhook();
        CoralFansVillageRemoveHook::unhook();
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