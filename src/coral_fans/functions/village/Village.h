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
    std::weak_ptr<Village>     mVillagePtr;
    int                        mDimId = -1;
    Tick                       mLastTick;
    AABB                       mBounds;
    AABB                       mRaidBounds;
    bsci::GeometryGroup::GeoId mBoundsGeoId     = {0};
    bsci::GeometryGroup::GeoId mRaidBoundsGeoId = {0};
    bsci::GeometryGroup::GeoId mIronSpawnGeoId  = {0};
    bsci::GeometryGroup::GeoId mCenterGeoId     = {0};
    bsci::GeometryGroup::GeoId mPoiQueryGeoId   = {0};
    bsci::GeometryGroup::GeoId mBindGeoId       = {0};

public:
    CFTickingVillageData(const Village&, Tick&);

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
    std::vector<std::optional<std::weak_ptr<Village>>>                   mVillageList;
    std::unordered_map<mce::UUID, std::unique_ptr<CFTickingVillageData>> mTickingList;

public:
    bool                     mayCreatingVillage = false;
    std::optional<mce::UUID> newVillageId       = std::nullopt;

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
    void                         addVillage(std::weak_ptr<Village>);
    void                         handleVillageTick(const Village&, Tick&);
    void                         tick(const Tick&);
    std::vector<std::string>     listVillages();
    std::string                  listTickingVillages();
    std::pair<std::string, bool> getVillageInfo(int);
    int                          getVillageId(std::weak_ptr<Village>);

    std::pair<std::string, bool> getVillagerInfo(ActorUniqueID);
    void                         clear();

private:
    CFVillageManager() = default;

public:
    [[nodiscard]] static CFVillageManager& getInstance() {
        static CFVillageManager instance;
        return instance;
    }
    static void hookVillage(bool);

    [[nodiscard]] static int getBedPOICount(std::shared_ptr<Village> villagePtr);
};
} // namespace coral_fans::functions