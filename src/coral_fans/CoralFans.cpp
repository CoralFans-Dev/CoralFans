#include "coral_fans/CoralFans.h"

#include <memory>

#include "ll/api/mod/RegisterHelper.h"

#include "coral_fans/base/Mod.h"
#include "coral_fans/commands/Commands.h"

namespace coral_fans {

static std::unique_ptr<CoralFans> instance;

CoralFans& CoralFans::getInstance() { return *instance; }

bool CoralFans::load() {
    getSelf().getLogger().debug("Loading...");

    // load Config Database
    const auto& configDbPath        = getSelf().getDataDir() / "config";
    coral_fans::mod().getConfigDb() = std::make_unique<ll::data::KeyValueDB>(configDbPath);

    return true;
}

bool CoralFans::enable() {
    getSelf().getLogger().debug("Enabling...");

    // register commands
    coral_fans::commands::registerTickCommand();
    coral_fans::commands::registerFuncCommand();
    coral_fans::commands::registerSelfCommand();

    return true;
}

bool CoralFans::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace coral_fans

LL_REGISTER_MOD(coral_fans::CoralFans, coral_fans::instance);
