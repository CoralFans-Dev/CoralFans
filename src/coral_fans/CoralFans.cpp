#include "coral_fans/CoralFans.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/commands/Commands.h"
#include "coral_fans/functions/func/FuncManager.h"
#include "coral_fans/functions/shortcuts/Shortcuts.h"
#include "ll/api/Config.h"
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

    // register commands
    commands::registerCoralfansCommand();
    if (getConfig().command.tick.enabled) commands::registerTickCommand(getConfig().command.tick.permission);
    if (getConfig().command.func.enabled) commands::registerFuncCommand(getConfig().command.func.permission);
    if (getConfig().command.self.enabled) commands::registerSelfCommand(getConfig().command.self.permission);
    if (getConfig().command.hsa.enabled) commands::registerHsaCommand(getConfig().command.hsa.permission);
    if (getConfig().command.counter.enabled) commands::registerCounterCommand(getConfig().command.counter.permission);
    if (getConfig().command.prof.enabled) commands::registerProfCommand(getConfig().command.prof.permission);
    if (getConfig().command.slime.enabled) commands::registerSlimeCommand(getConfig().command.slime.permission);
    if (getConfig().command.village.enabled) commands::registerVillageCommand(getConfig().command.village.permission);
    if (getConfig().command.rotate.enabled) commands::registerRotateCommand(getConfig().command.rotate.permission);
    if (getConfig().command.data.enabled) commands::registerDataCommand(getConfig().command.data.permission);
    if (getConfig().command.cfhud.enabled) commands::registerCfhudCommand(getConfig().command.cfhud.permission);
    if (getConfig().command.log.enabled) commands::registerLogCommand(getConfig().command.log.permission);
    if (getConfig().command.calculate.enabled)
        commands::registerCalculateCommand(getConfig().command.calculate.permission);
    if (getConfig().command.minerule.enabled)
        commands::registerMineruleCommand(getConfig().command.minerule.permission);
    if (getConfig().command.freecamera.enabled)
        commands::registerFreeCameraCommand(getConfig().command.freecamera.permission);
    if (getConfig().command.noclip.enabled) commands::registerNoclipCommand(getConfig().command.noclip.permission);
    if (getConfig().command.locate.enabled) commands::registerLocateCommand(getConfig().command.locate.permission);
    // register containerreader
    functions::registerContainerReader();
    // register shortcuts when first player join (确保其他所有插件的指令已被注册)
    functions::ShortcutsManager::getInstance().waitToRegisterShortcuts();

    return true;
}

bool CoralFans::disable() {
    getSelf().getLogger().debug("Disabling...");
    return true;
}

} // namespace coral_fans

LL_REGISTER_MOD(coral_fans::CoralFans, coral_fans::CoralFans::getInstance());
