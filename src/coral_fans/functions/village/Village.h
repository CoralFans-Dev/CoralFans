#pragma once

#include "bsci/GeometryGroup.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/platform/UUID.h"
#include "mc/world/actor/ai/village/Village.h"

#include <map>
#include <utility>


namespace coral_fans::functions {

class CFVillageManager {
private:
    std::map<std::string, int>              mUuidVidMap;
    int                                     mVidCounter = 0;
    std::map<int, std::pair<Village*, int>> mVidVillageMap;
    bsci::GeometryGroup::GeoId              mParticleId;

public:
    bool mShowBounds     = false;
    bool mShowRaidBounds = false;
    bool mShowIronSpawn  = false;
    bool mShowCenter     = false;
    bool mShowPoiQuery   = false;
    bool mShowBind       = false;

public:
    int                          getVid(mce::UUID);
    void                         insertVillage(Village*, int);
    void                         removeVillage(Village&);
    void                         clearParticle();
    void                         lightTick();
    void                         heavyTick();
    std::string                  listTickingVillages();
    void                         refreshCommandSoftEnum();
    std::pair<std::string, bool> getVillageInfo(std::string);
    std::pair<std::string, bool> getVillagerInfo(ActorUniqueID);

    static void hookVillage(bool);
};
} // namespace coral_fans::functions