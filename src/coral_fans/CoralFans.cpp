#include "coral_fans/CoralFans.h"

#include <memory>

#include "ll/api/mod/RegisterHelper.h"

#include "ll/api/Config.h"
#include "ll/api/i18n/I18n.h"

#include "coral_fans/base/Mod.h"
#include "coral_fans/commands/Commands.h"

namespace coral_fans {

static std::unique_ptr<CoralFans> instance;

CoralFans& CoralFans::getInstance() { return *instance; }

bool CoralFans::load() {
    const auto& logger = getSelf().getLogger();

    // load config
    const auto& configFilePath = getSelf().getConfigDir() / "config.json";
    if (!ll::config::loadConfig(coral_fans::mod().getConfig(), configFilePath)) {
        logger.warn("Cannot load configurations from {}", configFilePath);
        logger.info("Saving default configurations");
        if (!ll::config::saveConfig(coral_fans::mod().getConfig(), configFilePath)) {
            logger.error("Cannot save default configurations to {}", configFilePath);
            return false;
        }
    }

    // load i18n
    logger.debug("Loading I18n");
    ll::i18n::load(getSelf().getLangDir());
    ll::i18n::getInstance()->mDefaultLocaleName = coral_fans::mod().getConfig().locateName;

    // load Config Database
    logger.debug("Loading Config Database");
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
