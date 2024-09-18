#include "coral_fans/CoralFans.h"

#include <memory>

#include "bsci/GeometryGroup.h"

#include "ll/api/mod/RegisterHelper.h"

#include "ll/api/Config.h"
#include "ll/api/i18n/I18n.h"

#include "coral_fans/base/Mod.h"
#include "coral_fans/commands/Commands.h"
#include "coral_fans/functions/ContainerReader.h"
#include "coral_fans/functions/Shortcuts.h"

namespace coral_fans {

static std::unique_ptr<CoralFans> instance;

CoralFans& CoralFans::getInstance() { return *instance; }

bool CoralFans::load() {
    const auto& logger = getSelf().getLogger();
    auto&       mod    = coral_fans::mod();

    // load config
    try {
        const auto& configFilePath = getSelf().getConfigDir() / "config.json";
        if (!ll::config::loadConfig(mod.getConfig(), configFilePath)) {
            logger.warn("Cannot load configurations from {}", configFilePath);
            logger.info("Saving default configurations");
            if (!ll::config::saveConfig(mod.getConfig(), configFilePath)) {
                logger.error("Cannot save default configurations to {}", configFilePath);
                return false;
            }
        }
    } catch (...) {
        logger.error("Failed to load config.json. Please check the file!");
        return false;
    }


    // load i18n
    logger.debug("Loading I18n");
    ll::i18n::load(getSelf().getLangDir());
    ll::i18n::getInstance()->mDefaultLocaleName = mod.getConfig().locateName;

    // load Config Database
    logger.debug("Loading Config Database");
    const auto& configDbPath = getSelf().getDataDir() / "config";
    mod.getConfigDb()        = std::make_unique<ll::data::KeyValueDB>(configDbPath);

    // load GeometryGroup
    mod.getGeometryGroup() = bsci::GeometryGroup::createDefault();

    return true;
}

bool CoralFans::enable() {
    getSelf().getLogger().debug("Enabling...");
    auto& mod = coral_fans::mod();

    // register commands
    coral_fans::commands::registerCoralfansCommand();
    if (mod.getConfig().command.tick.enabled)
        coral_fans::commands::registerTickCommand(mod.getConfig().command.tick.permission);
    if (mod.getConfig().command.func.enabled)
        coral_fans::commands::registerFuncCommand(mod.getConfig().command.func.permission);
    if (mod.getConfig().command.self.enabled)
        coral_fans::commands::registerSelfCommand(mod.getConfig().command.self.permission);
    if (mod.getConfig().command.hsa.enabled)
        coral_fans::commands::registerHsaCommand(mod.getConfig().command.hsa.permission);
    if (mod.getConfig().command.counter.enabled)
        coral_fans::commands::registerCounterCommand(mod.getConfig().command.counter.permission);
    if (mod.getConfig().command.prof.enabled)
        coral_fans::commands::registerProfCommand(mod.getConfig().command.prof.permission);
    if (mod.getConfig().command.slime.enabled)
        coral_fans::commands::registerSlimeCommand(mod.getConfig().command.slime.permission);
    if (mod.getConfig().command.village.enabled)
        coral_fans::commands::registerVillageCommand(mod.getConfig().command.village.permission);
    if (mod.getConfig().command.rotate.enabled)
        coral_fans::commands::registerRotateCommand(mod.getConfig().command.rotate.permission);
    if (mod.getConfig().command.data.enabled)
        coral_fans::commands::registerDataCommand(mod.getConfig().command.data.permission);

    // register shortcuts
    coral_fans::functions::registerShortcutsListener();
    coral_fans::functions::registerShortcutsCommand();

    // register containerreader
    coral_fans::functions::registerContainerReader();

    return true;
}

bool CoralFans::disable() {
    getSelf().getLogger().debug("Disabling...");
    auto& mod = coral_fans::mod();

    // remove listeners
    for (const auto& listener : mod.getEventListeners()) mod.getEventBus().removeListener(listener);

    // remove tasks
    mod.getTickScheduler().clear();

    return true;
}

} // namespace coral_fans

LL_REGISTER_MOD(coral_fans::CoralFans, coral_fans::instance);
