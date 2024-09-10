#pragma once

#include "bsci/GeometryGroup.h"
#include "coral_fans/Config.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/functions/HopperCounter.h"
#include "coral_fans/functions/Hsa.h"
#include "coral_fans/functions/Prof.h"
#include "ll/api/Logger.h"
#include "ll/api/data/KeyValueDB.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include <memory>
#include <set>


namespace coral_fans {

class CoralFansMod {
private:
    std::unique_ptr<ll::data::KeyValueDB> mConfigDb;
    Config                                mConfig;
    std::unique_ptr<bsci::GeometryGroup>  mGeometryGroup;
    functions::HsaManager                 mHsaManager;
    functions::HopperCounterManager       mHopperCounterManager;
    ll::event::EventBus*                  mEventBus;
    std::set<ll::event::ListenerPtr>      mEventListeners;
    functions::Profiler                   mProfiler;

public:
    CoralFansMod() : mEventBus(&ll::event::EventBus::getInstance()) {}
    void init();

public:
    inline std::unique_ptr<ll::data::KeyValueDB>& getConfigDb() { return this->mConfigDb; }
    inline Config&                                getConfig() { return this->mConfig; }
    inline std::unique_ptr<bsci::GeometryGroup>&  getGeometryGroup() { return this->mGeometryGroup; }
    inline functions::HsaManager&                 getHsaManager() { return this->mHsaManager; }
    inline functions::HopperCounterManager&       getHopperCounterManager() { return this->mHopperCounterManager; }
    inline const ll::Logger&                      getLogger() { return CoralFans::getInstance().getSelf().getLogger(); }
    inline ll::event::EventBus*&                  getEventBus() { return this->mEventBus; }
    inline std::set<ll::event::ListenerPtr>&      getEventListeners() { return this->mEventListeners; }
    inline functions::Profiler&                   getProfiler() { return this->mProfiler; }

public:
    void lightTick();
    void heavyTick();

public:
    const std::string VERSION = "0.0.1";
};

CoralFansMod& mod();

} // namespace coral_fans