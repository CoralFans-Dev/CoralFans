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
// #pragma once

// #include "bsci/GeometryGroup.h"
// #include "mc/legacy/ActorUniqueID.h"
// #include "mc/platform/UUID.h"
// #include "mc/world/actor/ai/village/Village.h"


// #include <map>
// #include <utility>

// namespace coral_fans::functions {

// class CFVillageManager {
// private:
//     std::map<mce::UUID, int> mUuidVidMap;
//     int                      mVidCounter = 0;
//     // std::map<int, std::pair<Village*, int>> mVidVillageMap;
//     bsci::GeometryGroup::GeoId mParticleId;
//     bool                       mShowBounds;
//     bool                       mShowRaidBounds;
//     bool                       mShowIronSpawn;
//     bool                       mShowCenter;
//     bool                       mShowPoiQuery;
//     bool                       mShowBind;

// public:
//     int                          getVid(mce::UUID);
//     void                         insertVillage(Village*, int);
//     void                         clearParticle();
//     inline void                  setShowBounds(bool enable) { this->mShowBounds = enable; }
//     inline void                  setShowRaidBounds(bool enable) { this->mShowRaidBounds = enable; }
//     inline void                  setShowIronSpawn(bool enable) { this->mShowIronSpawn = enable; }
//     inline void                  setShowCenter(bool enable) { this->mShowCenter = enable; }
//     inline void                  setShowPoiQuery(bool enable) { this->mShowPoiQuery = enable; }
//     inline void                  setShowBind(bool enable) { this->mShowBind = enable; }
//     void                         lightTick();
//     void                         heavyTick();
//     std::string                  listTickingVillages();
//     void                         refreshCommandSoftEnum();
//     std::pair<std::string, bool> getVillageInfo(std::string);
//     std::pair<std::string, bool> getVillagerInfo(ActorUniqueID);
// };

// } // namespace coral_fans::functions