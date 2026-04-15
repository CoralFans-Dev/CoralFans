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
    AABB                       mOldRaidBounds;
    bsci::GeometryGroup::GeoId mBoundsGeoId     = {0};
    bsci::GeometryGroup::GeoId mRaidBoundsGeoId = {0};
    bsci::GeometryGroup::GeoId mIronSpawnGeoId  = {0};
    bsci::GeometryGroup::GeoId mCenterGeoId     = {0};
    bsci::GeometryGroup::GeoId mPoiQueryGeoId   = {0};
    bsci::GeometryGroup::GeoId mBindGeoId       = {0};

public:
    CFTickingVillageData(Village*, Tick&);

public:
    void showBounds();
    void showRaidBounds();
    void showIronSpawn();
    void showCenter();
    void showPoiQuery();
    void showBind();
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

private:
    bool mShowBounds     = false;
    bool mShowRaidBounds = false;
    bool mShowIronSpawn  = false;
    bool mShowCenter     = false;
    bool mShowPoiQuery   = false;
    bool mShowBind       = false;

public:
    void setShowBounds(bool);
    void setShowRaidBounds(bool);
    void setShowIronSpawn(bool);
    void setShowCenter(bool);
    void setShowPoiQuery(bool);
    void setShowBind(bool);
    bool getShowBounds();
    bool getShowRaidBounds();
    bool getShowIronSpawn();
    bool getShowCenter();
    bool getShowPoiQuery();
    bool getShowBind();

public:
    void                         addVillage(Village*);
    void                         handleVillageTick(Village*, Tick&);
    void                         removeVillage(Village*);
    void                         tick(const Tick&);
    std::vector<std::string>     listVillages();
    std::string                  listTickingVillages();
    std::pair<std::string, bool> getVillageInfo(int);
    int                          getVillageId(Village*);

    // void                         insertVillage(Village*, int);
    // void                         clearParticle();
    std::pair<std::string, bool> getVillagerInfo(ActorUniqueID);
    void                         clear();

public:
    static CFVillageManager& getInstance() {
        static CFVillageManager instance;
        return instance;
    }
    static void hookVillage(bool);
};
} // namespace coral_fans::functions