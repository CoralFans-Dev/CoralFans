#include "coral_fans/functions/village/Village.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
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
#include <math.h>

#include <array>
#include <format>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace {

struct CFAUIDCmp {
    bool operator()(const ActorUniqueID& a, const ActorUniqueID& b) const { return a.getHash() == b.getHash(); }
};

/*
version: BDS 1.21.3.01
from Village::_tryAddPoiToVillage(ActorUniqueID const &,std::weak_ptr<POIInstance>)
pseudocode line 108
(96 * 1)
*/
constexpr unsigned long long DWELLER_POI_MAP_OFFSET = 96;
// from trapdoor-ll
using DwellerPoiMapType =
    std::unordered_map<ActorUniqueID, std::array<std::weak_ptr<POIInstance>, 3>, std::hash<ActorUniqueID>, CFAUIDCmp>;
inline DwellerPoiMapType& getDwellerPoiMap(Village* v) {
    return ll::memory::dAccess<DwellerPoiMapType>(v, DWELLER_POI_MAP_OFFSET);
}

/*
version: BDS 1.21.3.01
from Village::addVillager(PlayerInventory **this, const struct ActorUniqueID *a2)
pseudocode line 61
(20 * 8)
*/
constexpr unsigned long long DWELLER_TICK_MAP_OFFSET = 160;
// from trapdoor-ll
using DwellerTickMapType = std::array<std::unordered_map<ActorUniqueID, Village::DwellerData>, 4>;
inline DwellerTickMapType& getDwellerTickMap(Village* v) {
    return ll::memory::dAccess<DwellerTickMapType>(v, DWELLER_TICK_MAP_OFFSET);
}

// from trapdoor-ll
inline std::array<unsigned long long, 4> getDwellerCount(Village* v) {
    auto& map = getDwellerTickMap(v);
    return {map[0].size(), map[1].size(), map[2].size(), map[3].size()};
}

} // namespace

namespace coral_fans::functions {
float getApproximateRadius(AABB& bound) {
    float x = bound.max.x - bound.min.x;
    float z = bound.max.z - bound.min.z;
    if (fabs(x - z) >= 0.00000011920929f) return (x + z) * 0.3f;
    else return 1.414213562373f * x;
}

int CFVillageManager::getVid(mce::UUID uuid) {
    if (auto it = this->mUuidVidMap.find(uuid); it != this->mUuidVidMap.end()) return it->second;
    this->mUuidVidMap[uuid] = this->mVidCounter++;
    return this->mVidCounter - 1;
}

void CFVillageManager::insertVillage(Village* village, int dimid) {
    if (!village) return;
    this->mVidVillageMap.insert({
        this->getVid(village->mUniqueID),
        {village, dimid}
    });
}

void CFVillageManager::removeVillage(Village& village) { this->mVidVillageMap.erase(this->getVid(village.mUniqueID)); }

void CFVillageManager::clearParticle() { coral_fans::mod().getGeometryGroup()->remove(this->mParticleId); }

void CFVillageManager::lightTick() {
    static int gt = 0;
    if (gt == 0) {
        this->refreshCommandSoftEnum();
        this->mVidVillageMap.clear();
    }
    gt = (gt + 1) % 20;
}

void CFVillageManager::heavyTick() {
    static int gt = 0;
    if (gt == 1) { // delay 1 tick
        this->clearParticle();
        auto& mod = coral_fans::mod();
        for (auto kv : this->mVidVillageMap) {
            if (!kv.second.first) continue;
            if (this->mShowBounds) {
                auto ids = std::array{
                    this->mParticleId,
                    mod.getGeometryGroup()->box(kv.second.second, kv.second.first->mBounds)
                };
                this->mParticleId = mod.getGeometryGroup()->merge(ids);
            }
            if (this->mShowRaidBounds) {
                auto ids = std::array{
                    this->mParticleId,
                    mod.getGeometryGroup()
                        ->box(kv.second.second, kv.second.first->mStaticRaidBounds, mce::Color::ORANGE())
                };
                this->mParticleId = mod.getGeometryGroup()->merge(ids);
            }
            if (this->mShowIronSpawn) {
                auto ids = std::array{
                    this->mParticleId,
                    mod.getGeometryGroup()->box(
                        kv.second.second,
                        {kv.second.first->mBounds->center() - Vec3{8, 6, 8},
                                        kv.second.first->mBounds->center() + Vec3{9, 7, 9}},
                        mce::Color::BLUE()
                    )
                };
                this->mParticleId = mod.getGeometryGroup()->merge(ids);
            }
            if (this->mShowCenter) {
                auto ids = std::array{
                    this->mParticleId,
                    mod.getGeometryGroup()->box(
                        kv.second.second,
                        {kv.second.first->mBounds->center(), kv.second.first->mBounds->center() + Vec3{1, 1, 1}},
                        mce::Color::RED()
                    )
                };
                this->mParticleId = mod.getGeometryGroup()->merge(ids);
            }
            if (this->mShowPoiQuery) {
                auto ids = std::array{
                    this->mParticleId,
                    mod.getGeometryGroup()->box(
                        kv.second.second,
                        {kv.second.first->mBounds->min - Vec3{64, 64, 64},
                                        kv.second.first->mBounds->max + Vec3{64, 64, 64}},
                        mce::Color::PINK()
                    )
                };
                this->mParticleId = mod.getGeometryGroup()->merge(ids);
            }
            if (this->mShowBind) {
                auto level = ll::service::getLevel();
                if (level) {
                    auto                    ids = std::vector{this->mParticleId};
                    const static mce::Color colors[3] =
                        {mce::Color::CYAN(), mce::Color::MINECOIN_GOLD(), mce::Color::GREEN()};
                    for (auto& item : ::getDwellerPoiMap(kv.second.first)) {
                        auto villager = level->fetchEntity(item.first, false);
                        if (villager) {
                            for (int i = 0; i < 3; i++) {
                                const auto& poi = item.second[i].lock();
                                if (poi) {
                                    ids.emplace_back(mod.getGeometryGroup()->line(
                                        villager->getDimensionId(),
                                        villager->getHeadPos(),
                                        poi->mPosition->center(),
                                        colors[i]
                                    ));
                                }
                            }
                        }
                    }
                    this->mParticleId = mod.getGeometryGroup()->merge(ids);
                }
            }
        }
    }
    gt = (gt + 1) % 40;
}

std::string CFVillageManager::listTickingVillages() {
    std::string retstr;
    for (auto& kv : this->mVidVillageMap) {
        if (!kv.second.first) continue;
        auto  dwellerCountArray  = ::getDwellerCount(kv.second.first);
        float approximateRadius  = getApproximateRadius(*kv.second.first->mBounds);
        retstr                  += std::format(
            "- §a[{}]§r §b[{}]§r r: {} p: {} g: {} b: {} §6[{}, {}]§r\n",
            kv.first,
            kv.second.first->mBounds->center(),
            approximateRadius,
            dwellerCountArray[0], // Villager
            dwellerCountArray[1], // IronGolem
            kv.second.first->getBedPOICount(),
            kv.second.first->mBounds->min,
            kv.second.first->mBounds->max
        );
    }
    return retstr;
}

// from trapdoor-ll
void CFVillageManager::refreshCommandSoftEnum() {
    std::vector<std::string> vids;
    std::vector<std::string> uuids;
    for (auto& v : this->mVidVillageMap) {
        if (v.second.first) {
            vids.push_back(std::to_string(v.first));
            uuids.push_back(v.second.first->mUniqueID->asString().substr(0, 8) + "...");
        }
    }
    vids.insert(vids.end(), uuids.begin(), uuids.end());
    ll::command::CommandRegistrar::getInstance().setSoftEnumValues("villageid", vids);
}

std::pair<std::string, bool> CFVillageManager::getVillageInfo(std::string id) {
    using ll::i18n_literals::operator""_tr;
    int vid;
    if (id.size() == 11) vid = this->mUuidVidMap[id.substr(0, 8)];
    else vid = std::stoi(id);
    Village* village = this->mVidVillageMap[vid].first;
    if (!village) return {"translate.village.cannotget"_tr(), false};
    auto        dwellerCountArray = ::getDwellerCount(village);
    float       approximateRadius = getApproximateRadius(*village->mBounds);
    std::string retstr            = "translate.village.info"_tr(
        vid,
        village->mUniqueID->asString(),
        village->mBounds->center().toString(),
        village->mBounds->min.toString(),
        village->mBounds->max.toString(),
        approximateRadius,
        dwellerCountArray[0],
        dwellerCountArray[1],
        dwellerCountArray[2],
        dwellerCountArray[3]
    );
    for (auto& villager : ::getDwellerPoiMap(village)) {
        for (int index = 0; index < 3; ++index) {
            if (index == 0) {
                retstr += "|";
            }
            auto poi = villager.second[index].lock();
            if (poi) retstr += std::format(" §a{}§r, §e{:.1f}§r |", *poi->mPosition, poi->mRadius);
            else retstr += " §7(x)§r |";
            if (index == 2) retstr += "\n";
        }
    }
    return {retstr, true};
}

std::pair<std::string, bool> CFVillageManager::getVillagerInfo(ActorUniqueID auid) {
    using ll::i18n_literals::operator""_tr;
    std::string retstr;
    for (auto v : this->mVidVillageMap) {
        auto dwellerPoiMap = ::getDwellerPoiMap(v.second.first);
        auto it            = dwellerPoiMap.find(auid);
        if (it != dwellerPoiMap.end()) {
            retstr += "VID: " + std::to_string(v.first);
            for (int i = 0; i < 3; i++) {
                const auto& poi = it->second[i].lock();
                if (poi) retstr += std::format("\n{}: {}, {:.2f}", poi->mName->mStr, *poi->mPosition, poi->mRadius);
            }
            return {retstr, true};
        }
    }
    return {"translate.village.cannotget"_tr(), false};
}

// hook village tick
LL_TYPE_INSTANCE_HOOK(
    CoralFansVillageTickHook,
    ll::memory::HookPriority::Normal,
    Village,
    &Village::tick,
    void,
    Tick         tick,
    BlockSource& region
) {
    coral_fans::mod().getVillageManager().insertVillage(this, region.getDimensionId());
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
    coral_fans::mod().getVillageManager().removeVillage(village);
    origin(village);
}

void hookVillage(bool hook) {
    if (hook) {
        CoralFansVillageTickHook::hook();
        CoralFansVillageRemoveHook::hook();
    } else {
        CoralFansVillageTickHook::unhook();
        CoralFansVillageRemoveHook::unhook();
    }
}

} // namespace coral_fans::functions