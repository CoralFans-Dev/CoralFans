#include "coral_fans/CoralFans.h"

#include <memory>

#include "ll/api/mod/RegisterHelper.h"

namespace coral_fans {

static std::unique_ptr<CoralFans> instance;

CoralFans& CoralFans::getInstance() { return *instance; }

bool CoralFans::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool CoralFans::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    return true;
}

bool CoralFans::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace coral_fans

LL_REGISTER_MOD(coral_fans::CoralFans, coral_fans::instance);
