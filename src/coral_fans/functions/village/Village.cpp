#include "coral_fans/functions/village/Village.h"
#include "Village.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Color.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/platform/UUID.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ai/village/POIInstance.h"
#include "mc/world/actor/ai/village/Village.h"
#include "mc/world/actor/ai/village/VillageManager.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Tick.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/phys/AABB.h"
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

// int CFVillageManager::getVid(mce::UUID uuid) {
//     if (auto it = this->mUuidVidMap.find(uuid.asString()); it != this->mUuidVidMap.end()) return it->second;
//     this->mUuidVidMap[uuid.asString()] = this->mVidCounter++;
//     return this->mVidCounter - 1;
// }

// void CFVillageManager::insertVillage(Village* village, int dimid) {
//     if (!village) return;
//     this->mVidVillageMap.insert({
//         this->getVid(village->mUniqueID),
//         {village, dimid}
//     });
// }

CFTickingVillageData::CFTickingVillageData(Village* villagePtr, Tick& tick, AABB& bounds) {
    this->mVillagePtr = villagePtr;
    this->mLastTick   = tick;
    this->mOldBounds  = bounds;
}

void CFVillageManager::addVillage(Village* villagePtr) {
    this->mVillageList.emplace_back(villagePtr);
    ll::command::CommandRegistrar::getInstance(false).addSoftEnumValues(
        "villageid",
        {std::to_string(this->mVillageList.size())}
    );
}

void CFVillageManager::handleVillageTick(Village* villagePtr, Tick& tick, AABB& bounds) {
    auto [it, isInserted] = this->mTickingList.try_emplace(villagePtr->mUniqueID);
    if (isInserted) it->second = std::make_unique<CFTickingVillageData>(villagePtr, tick, bounds);
    else {
        it->second->mLastTick = tick;
    }
}

void CFVillageManager::removeVillage(Village* villagePtr) {
    auto it = this->mTickingList.find(villagePtr->mUniqueID);
    if (it != this->mTickingList.end() && it->second) {
        if (this->mShowBounds) coral_fans::mod().getGeometryGroup()->remove(it->second->mBoundsGeoId);
        if (this->mShowRaidBounds) coral_fans::mod().getGeometryGroup()->remove(it->second->mRaidBoundsGeoId);
        if (this->mShowIronSpawn) coral_fans::mod().getGeometryGroup()->remove(it->second->mIronSpawnGeoId);
        if (this->mShowCenter) coral_fans::mod().getGeometryGroup()->remove(it->second->mCenterGeoId);
        if (this->mShowPoiQuery) coral_fans::mod().getGeometryGroup()->remove(it->second->mPoiQueryGeoId);
        if (this->mShowBind) coral_fans::mod().getGeometryGroup()->remove(it->second->mBindGeoId);
    }
    this->mTickingList.erase(it);
    auto size = this->mVillageList.size();
    for (size_t i = 0; i < size; i++) {
        if (mVillageList[i] && mVillageList[i] == villagePtr) {
            mVillageList[i] = nullptr;
            ll::command::CommandRegistrar::getInstance(false).removeSoftEnumValues(
                "villageid",
                {std::to_string(i + 1)}
            );
            break;
        }
    }
}

void CFVillageManager::tick() {}

// void CFVillageManager::clearParticle() { coral_fans::mod().getGeometryGroup()->remove(this->mParticleId); }

// void CFVillageManager::heavyTick() {
//     static int gt = 0;
// if (gt == 1) { // delay 1 tick
//     this->clearParticle();
//     auto& mod = coral_fans::mod();
//     for (auto kv : this->mVidVillageMap) {
//         if (!kv.second.first) continue;
//         if (this->mShowBounds) {
//             auto ids = std::array{
//                 this->mParticleId,
//                 mod.getGeometryGroup()->box(kv.second.second, kv.second.first->mBounds)
//             };
//             this->mParticleId = mod.getGeometryGroup()->merge(ids);
//         }
//         if (this->mShowRaidBounds) {
//             auto ids = std::array{
//                 this->mParticleId,
//                 mod.getGeometryGroup()
//                     ->box(kv.second.second, kv.second.first->mStaticRaidBounds, mce::Color::REBECCA_PURPLE())
//             };
//             this->mParticleId = mod.getGeometryGroup()->merge(ids);
//         }
//         if (this->mShowIronSpawn) {
//             auto ids = std::array{
//                 this->mParticleId,
//                 mod.getGeometryGroup()->box(
//                     kv.second.second,
//                     {kv.second.first->mBounds->center() - Vec3{8, 6, 8},
//                                     kv.second.first->mBounds->center() + Vec3{9, 7, 9}},
//                     mce::Color::BLUE()
//                 )
//             };
//             this->mParticleId = mod.getGeometryGroup()->merge(ids);
//         }
//         if (this->mShowCenter) {
//             auto ids = std::array{
//                 this->mParticleId,
//                 mod.getGeometryGroup()->box(
//                     kv.second.second,
//                     {kv.second.first->mBounds->center(), kv.second.first->mBounds->center() + Vec3{1, 1, 1}},
//                     mce::Color::RED()
//                 )
//             };
//             this->mParticleId = mod.getGeometryGroup()->merge(ids);
//         }
//         if (this->mShowPoiQuery) {
//             auto ids = std::array{
//                 this->mParticleId,
//                 mod.getGeometryGroup()->box(
//                     kv.second.second,
//                     {kv.second.first->mBounds->min - Vec3{64, 64, 64},
//                                     kv.second.first->mBounds->max + Vec3{64, 64, 64}},
//                     mce::Color::PINK()
//                 )
//             };
//             this->mParticleId = mod.getGeometryGroup()->merge(ids);
//         }
//         if (this->mShowBind) {
//             auto level = ll::service::getLevel();
//             if (level) {
//                 auto                    ids = std::vector{this->mParticleId};
//                 const static mce::Color colors[3] =
//                     {mce::Color::PURPLE(), mce::Color::WHITE(), mce::Color::GREEN()};
//                 for (auto& item : ::getDwellerPoiMap(kv.second.first)) {
//                     auto villager = level->fetchEntity(item.first, false);
//                     if (villager) {
//                         for (int i = 0; i < 3; i++) {
//                             const auto& poi = item.second[i].lock();
//                             if (poi) {
//                                 ids.emplace_back(mod.getGeometryGroup()->line(
//                                     villager->getDimensionId(),
//                                     villager->getHeadPos(),
//                                     poi->mPosition->center(),
//                                     colors[i]
//                                 ));
//                             }
//                         }
//                     }
//                 }
//                 this->mParticleId = mod.getGeometryGroup()->merge(ids);
//             }
//         }
//     }
// }
// gt = (gt + 1) % 40;
// }

std::string CFVillageManager::listVillages() {
    std::string retstr;
    // for (auto& kv : this->mVidVillageMap) {
    //     if (!kv.second.first) continue;
    //     auto  dwellerCountArray  = ::getDwellerCount(kv.second.first);
    //     float approximateRadius  = getApproximateRadius(*kv.second.first->mBounds);
    //     retstr                  += std::format(
    //         "- §a[{}]§r §b{}§r r: {} p: {} g: {} b: {} §6[{}, {}]§r\n",
    //         kv.first,
    //         kv.second.first->mBounds->center().toString(),
    //         approximateRadius,
    //         dwellerCountArray[0], // Villager
    //         dwellerCountArray[1], // IronGolem
    //         kv.second.first->getBedPOICount(),
    //         kv.second.first->mBounds->min.toString(),
    //         kv.second.first->mBounds->max.toString()
    //     );
    // }
    return retstr;
}

std::pair<std::string, bool> CFVillageManager::getVillageInfo(std::string id) {
    using ll::i18n_literals::operator""_tr;
    // int vid;
    // if (id[0] != '"') vid = this->mUuidVidMap[id];
    // else vid = std::stoi(id);
    // Village* village = this->mVidVillageMap[vid].first;
    // if (!village) return {"translate.village.cannotget"_tr(), false};
    // auto        dwellerCountArray = ::getDwellerCount(village);
    // float       approximateRadius = getApproximateRadius(*village->mBounds);
    // std::string retstr            = "translate.village.info"_tr(
    //     vid,
    //     village->mUniqueID->asString(),
    //     village->mBounds->center().toString(),
    //     village->mBounds->min.toString(),
    //     village->mBounds->max.toString(),
    //     approximateRadius,
    //     dwellerCountArray[0],
    //     dwellerCountArray[1],
    //     dwellerCountArray[2],
    //     dwellerCountArray[3]
    // );
    // int  i     = 0;
    // auto level = ll::service::getLevel();
    // for (auto& villager : ::getDwellerPoiMap(village)) {
    //     i++;
    //     retstr +=
    //         "translate.village.villagerInfo"_tr(i, level->fetchEntity(villager.first,
    //         false)->getFeetPos().toString());
    //     retstr    += "translate.village.villagerPOIType1"_tr();
    //     auto poi1  = villager.second[0].lock();
    //     if (poi1)
    //         retstr += "translate.village.villagerPOIInfo"_tr(
    //             *poi1->mPosition,
    //             poi1->mOwnerCount,
    //             poi1->mOwnerCapacity,
    //             poi1->mRadius,
    //             poi1->mWeight
    //         );
    //     else retstr += " §7(x)§r\n";
    //     retstr    += "translate.village.villagerPOIType2"_tr();
    //     auto poi2  = villager.second[1].lock();
    //     if (poi2)
    //         retstr += "translate.village.villagerPOIInfo"_tr(
    //             *poi2->mPosition,
    //             poi2->mOwnerCount,
    //             poi2->mOwnerCapacity,
    //             poi2->mRadius,
    //             poi2->mWeight
    //         );
    //     else retstr += " §7(x)§r\n";
    //     retstr    += "translate.village.villagerPOIType3"_tr();
    //     auto poi3  = villager.second[2].lock();
    //     if (poi3)
    //         retstr += "translate.village.villagerPOIInfo"_tr(
    //             *poi3->mPosition,
    //             poi3->mOwnerCount,
    //             poi3->mOwnerCapacity,
    //             poi3->mRadius,
    //             poi3->mWeight
    //         );
    //     else retstr += " §7(x)§r\n";
    // }
    // return {retstr, true};
    return {"", true};
}

std::pair<std::string, bool> CFVillageManager::getVillagerInfo(ActorUniqueID auid) {
    using ll::i18n_literals::operator""_tr;
    // std::string retstr;
    // for (auto v : this->mVidVillageMap) {
    //     auto dwellerPoiMap = ::getDwellerPoiMap(v.second.first);
    //     auto it            = dwellerPoiMap.find(auid);
    //     if (it != dwellerPoiMap.end()) {
    //         retstr += "VID: " + std::to_string(v.first);
    //         for (int i = 0; i < 3; i++) {
    //             const auto& poi = it->second[i].lock();
    //             if (poi) retstr += std::format("\n{}: {}, {:.2f}", poi->mName->mStr, *poi->mPosition, poi->mRadius);
    //         }
    //         return {retstr, true};
    //     }
    // }
    return {"translate.village.cannotget"_tr(), false};
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
    auto ori = origin(dimension, id, _origin);
    coral_fans::mod().getVillageManager().addVillage(static_cast<Village*>(ori));
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
    coral_fans::mod().getVillageManager().handleVillageTick(this, tick, this->mBounds);
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
    coral_fans::mod().getVillageManager().removeVillage(&village);
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
} // namespace coral_fans::functions