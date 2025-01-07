#pragma once

// #include "bsci/GeometryGroup.h"
#include "coral_fans/Config.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/functions/HopperCounter.h"
#include "coral_fans/functions/Hsa.h"
#include "coral_fans/functions/Hud.h"
#include "coral_fans/functions/Prof.h"
#include "coral_fans/functions/Slime.h"
#include "coral_fans/functions/Village.h"
#include "ll/api/data/KeyValueDB.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/io/Logger.h"
#include <memory>


namespace coral_fans {

class CoralFansMod {
private:
    std::unique_ptr<ll::data::KeyValueDB> mConfigDb;
    config::Config                        mConfig;
    std::unique_ptr<bsci::GeometryGroup>  mGeometryGroup;
    functions::HsaManager                 mHsaManager;
    functions::HopperCounterManager       mHopperCounterManager;
    std::set<ll::event::ListenerPtr>      mEventListeners;
    functions::Profiler                   mProfiler;
    functions::SlimeManager               mSlimeManager;
    functions::CFVillageManager           mVillageManager;
    functions::HudHelper                  mHudHelper;

public:
public:
    inline std::unique_ptr<ll::data::KeyValueDB>& getConfigDb() { return this->mConfigDb; }
    inline config::Config&                        getConfig() { return this->mConfig; }
    inline std::unique_ptr<bsci::GeometryGroup>&  getGeometryGroup() { return this->mGeometryGroup; }
    inline functions::HsaManager&                 getHsaManager() { return this->mHsaManager; }
    inline functions::HopperCounterManager&       getHopperCounterManager() { return this->mHopperCounterManager; }
    inline const ll::io::Logger&             getLogger() { return (CoralFans::getInstance().getSelf()).getLogger(); }
    inline ll::event::EventBus&              getEventBus() { return ll::event::EventBus::getInstance(); }
    inline std::set<ll::event::ListenerPtr>& getEventListeners() { return this->mEventListeners; }
    inline functions::Profiler&              getProfiler() { return this->mProfiler; }
    inline functions::SlimeManager&          getSlimeManager() { return this->mSlimeManager; }
    inline functions::CFVillageManager&      getVillageManager() { return this->mVillageManager; }

public:
    void tick();
};

CoralFansMod& mod();

} // namespace coral_fans