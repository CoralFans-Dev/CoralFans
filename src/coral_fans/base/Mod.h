#pragma once

#include "ll/api/data/KeyValueDB.h"

namespace coral_fans {

class CoralFansMod {
private:
    std::unique_ptr<ll::data::KeyValueDB> mConfigDb;

public:
    inline std::unique_ptr<ll::data::KeyValueDB>& getConfigDb() { return this->mConfigDb; };
};

CoralFansMod& mod();

} // namespace coral_fans