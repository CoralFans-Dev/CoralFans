#pragma once

#include "coral_fans/Config.h"
#include "ll/api/data/KeyValueDB.h"

namespace coral_fans {

class CoralFansMod {
private:
    std::unique_ptr<ll::data::KeyValueDB> mConfigDb;
    coral_fans::Config                    mConfig;

public:
    inline std::unique_ptr<ll::data::KeyValueDB>& getConfigDb() { return this->mConfigDb; }
    inline coral_fans::Config&                    getConfig() { return this->mConfig; }
};

CoralFansMod& mod();

} // namespace coral_fans