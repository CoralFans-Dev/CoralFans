#pragma once

#include "bsci/GeometryGroup.h"
#include "coral_fans/Config.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/functions/HopperCounter.h"
#include "coral_fans/functions/Hsa.h"
#include "ll/api/Logger.h"
#include "ll/api/data/KeyValueDB.h"
#include "ll/api/schedule/Scheduler.h"
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace coral_fans {

class CoralFansMod {
private:
    std::unique_ptr<ll::data::KeyValueDB> mConfigDb;
    Config                                mConfig;
    std::unique_ptr<bsci::GeometryGroup>  mGeometryGroup;
    ll::schedule::GameTickScheduler       mGameTickScheduler;
    std::unordered_map<std::string_view, std::shared_ptr<ll::schedule::Task<ll::chrono::GameTickClock>>> mTaskMap;
    functions::HsaManager                                                                                mHsaManager;
    functions::HopperCounterManager mHopperCounterManager;

public:
    void init();

public:
    inline std::unique_ptr<ll::data::KeyValueDB>& getConfigDb() { return this->mConfigDb; }
    inline Config&                                getConfig() { return this->mConfig; }
    inline std::unique_ptr<bsci::GeometryGroup>&  getGeometryGroup() { return this->mGeometryGroup; }
    inline functions::HsaManager&                 getHsaManager() { return this->mHsaManager; }
    inline functions::HopperCounterManager&       getHopperCounterManager() { return this->mHopperCounterManager; }
    inline const ll::Logger&                      getLogger() { return CoralFans::getInstance().getSelf().getLogger(); }

public:
    bool addTask(std::string_view, unsigned, std::function<void()>);
    bool removeTask(std::string_view);

public:
    const std::string VERSION = "0.0.1";
};

CoralFansMod& mod();

} // namespace coral_fans