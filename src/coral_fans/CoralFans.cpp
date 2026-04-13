#include "coral_fans/CoralFans.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/commands/Commands.h"
#include "coral_fans/functions/func/FuncManager.h"
#include "coral_fans/functions/shortcuts/Shortcuts.h"
#include "ll/api/Config.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ExecuteCommandEvent.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/RegisterHelper.h"
#include <memory>


namespace coral_fans {
struct CoralFans::Impl {
    config::Config                        mConfig;
    std::unique_ptr<ll::data::KeyValueDB> mConfigDb;
    std::unique_ptr<bsci::GeometryGroup>  mGeometryGroup;
    std::set<ll::event::ListenerPtr>      mEventListeners;
};

CoralFans::CoralFans() : impl(std::make_unique<Impl>()), mSelf(*ll::mod::NativeMod::current()) {}
CoralFans::~CoralFans() = default;

config::Config& CoralFans::getConfig() { return impl->mConfig; }

std::unique_ptr<ll::data::KeyValueDB>& CoralFans::getConfigDb() { return impl->mConfigDb; }

std::unique_ptr<bsci::GeometryGroup>& CoralFans::getGeometryGroup() { return impl->mGeometryGroup; }

std::set<ll::event::ListenerPtr>& CoralFans::getEventListeners() { return impl->mEventListeners; }

CoralFans& CoralFans::getInstance() {
    static CoralFans instance;
    return instance;
}

void CoralFans::setupCommands() {
    commands::registerCoralfansCommand();
    auto& commandsConfig = this->getConfig().command;
    if (commandsConfig.tick.enabled) commands::registerTickCommand(commandsConfig.tick.permission);
    if (commandsConfig.func.enabled) commands::registerFuncCommand(commandsConfig.func.permission);
    if (commandsConfig.self.enabled) commands::registerSelfCommand(commandsConfig.self.permission);
    if (commandsConfig.hsa.enabled) commands::registerHsaCommand(commandsConfig.hsa.permission);
    if (commandsConfig.counter.enabled) commands::registerCounterCommand(commandsConfig.counter.permission);
    if (commandsConfig.prof.enabled) commands::registerProfCommand(commandsConfig.prof.permission);
    if (commandsConfig.slime.enabled) commands::registerSlimeCommand(commandsConfig.slime.permission);
    if (commandsConfig.village.enabled) commands::registerVillageCommand(commandsConfig.village.permission);
    if (commandsConfig.rotate.enabled) commands::registerRotateCommand(commandsConfig.rotate.permission);
    if (commandsConfig.data.enabled) commands::registerDataCommand(commandsConfig.data.permission);
    if (commandsConfig.cfhud.enabled) commands::registerCfhudCommand(commandsConfig.cfhud.permission);
    if (commandsConfig.log.enabled) commands::registerLogCommand(commandsConfig.log.permission);
    if (commandsConfig.calculate.enabled) commands::registerCalculateCommand(commandsConfig.calculate.permission);
    if (commandsConfig.minerule.enabled) commands::registerMineruleCommand(commandsConfig.minerule.permission);
    if (commandsConfig.freecamera.enabled) commands::registerFreeCameraCommand(commandsConfig.freecamera.permission);
    if (commandsConfig.noclip.enabled) commands::registerNoclipCommand(commandsConfig.noclip.permission);
    if (commandsConfig.locate.enabled) commands::registerLocateCommand(commandsConfig.locate.permission);

    functions::ShortcutsManager::getInstance().registerShortcutsCommand();
}

bool CoralFans::load() {
    const auto& logger = getSelf().getLogger();

    // load config
    try {
        const auto& configFilePath = getSelf().getConfigDir() / "config.json";
        if (!ll::config::loadConfig(getConfig(), configFilePath)) {
            logger.warn("Cannot load configurations from {}", configFilePath);
            logger.info("Saving default configurations");
            if (!ll::config::saveConfig(getConfig(), configFilePath)) {
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
    if (!ll::i18n::getInstance().load(getSelf().getLangDir())) logger.error("Failed to load I18n");

    // load Config Database
    logger.debug("Loading Config Database");
    const auto& configDbPath = getSelf().getDataDir() / "config";
    getConfigDb()            = std::make_unique<ll::data::KeyValueDB>(configDbPath);

    // load GeometryGroup
    getGeometryGroup() = bsci::GeometryGroup::createDefault();


    return true;
}

bool CoralFans::enable() {
    const auto& logger = getSelf().getLogger();
    logger.debug("Enabling...");

    setupCommands();
    // register containerreader
    functions::registerContainerReader();
    functions::ShortcutsManager::getInstance().registerShortcutsListener();
    return true;
}

bool CoralFans::disable() {
    getSelf().getLogger().debug("Disabling...");
    return true;
}

} // namespace coral_fans

LL_REGISTER_MOD(coral_fans::CoralFans, coral_fans::CoralFans::getInstance());
