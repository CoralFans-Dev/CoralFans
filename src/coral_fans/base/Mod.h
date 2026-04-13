#pragma once

#include "bsci/GeometryGroup.h"
#include "coral_fans/Config.h"
#include "coral_fans/CoralFans.h"
#include "mc/world/level/Tick.h"


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
    std::set<ll::event::ListenerPtr>      mEventListeners;

public:
    inline std::unique_ptr<ll::data::KeyValueDB>& getConfigDb() { return this->mConfigDb; }
    inline config::Config&                        getConfig() { return this->mConfig; }
    inline std::unique_ptr<bsci::GeometryGroup>&  getGeometryGroup() { return this->mGeometryGroup; }
    inline const ll::io::Logger&             getLogger() { return (CoralFans::getInstance().getSelf()).getLogger(); }
    inline ll::event::EventBus&              getEventBus() { return ll::event::EventBus::getInstance(); }
    inline std::set<ll::event::ListenerPtr>& getEventListeners() { return this->mEventListeners; }

public:
    void tick(const Tick&);
};

CoralFansMod& mod();

} // namespace coral_fans