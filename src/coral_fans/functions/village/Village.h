#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/platform/UUID.h"
#include "mc/world/actor/ai/village/Village.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>


namespace coral_fans::functions {


class CFTickingVillageData {
public:
    Village*                   mVillagePtr;
    Tick                       mLastTick;
    AABB                       mOldBounds;
    bsci::GeometryGroup::GeoId mBoundsGeoId;
    bsci::GeometryGroup::GeoId mRaidBoundsGeoId;
    bsci::GeometryGroup::GeoId mIronSpawnGeoId;
    bsci::GeometryGroup::GeoId mCenterGeoId;
    bsci::GeometryGroup::GeoId mPoiQueryGeoId;
    bsci::GeometryGroup::GeoId mBindGeoId;

public:
    CFTickingVillageData(Village*, Tick&, AABB&);
};

class CFVillageManager {
private:
    // std::map<std::string, int>              mUuidVidMap;
    // int                                     mVidCounter = 0;
    std::vector<Village*> mVillageList;
    // std::unordered_map<mce::UUID, std::weak_ptr<CFVillageData>> mVillageDataMap;
    std::unordered_map<mce::UUID, std::unique_ptr<CFTickingVillageData>> mTickingList;
    // std::map<int, std::pair<Village*, int>>              mVidVillageMap;
    bsci::GeometryGroup::GeoId mParticleId;

public:
    bool mShowBounds     = false;
    bool mShowRaidBounds = false;
    bool mShowIronSpawn  = false;
    bool mShowCenter     = false;
    bool mShowPoiQuery   = false;
    bool mShowBind       = false;

public:
    void addVillage(Village*);
    void handleVillageTick(Village*, Tick&, AABB&);
    void removeVillage(Village*);
    void tick();

    // int                          getVid(mce::UUID);
    // void                         insertVillage(Village*, int);
    // void                         clearParticle();
    // void                         lightTick();
    // void                         heavyTick();
    std::string listVillages();
    // void                         refreshCommandSoftEnum();
    std::pair<std::string, bool> getVillageInfo(std::string);
    std::pair<std::string, bool> getVillagerInfo(ActorUniqueID);

    static void hookVillage(bool);
};
} // namespace coral_fans::functions