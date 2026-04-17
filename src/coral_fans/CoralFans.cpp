#include "coral_fans/CoralFans.h"
#include "base/MySchedule.h"
#include "bsci/GeometryGroup.h"
#include "coral_fans/commands/Commands.h"
#include "coral_fans/functions/freeCamera/FreeCamera.h"
#include "coral_fans/functions/func/FuncManager.h"
#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/functions/locate/DuplicatableManager.h"
#include "coral_fans/functions/minerule/MineruleManager.h"
#include "coral_fans/functions/noclip/Noclip.h"
#include "coral_fans/functions/shortcuts/Shortcuts.h"
#include "coral_fans/functions/slime/Slime.h"
#include "coral_fans/functions/village/Village.h"
#include "ll/api/Config.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ServerCommandRegisterEvent.h"
#include "ll/api/event/server/ServerStoppingEvent.h"
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
    if (commandsConfig.tick.enabled) commands::registerTickCommand(commandsConfig.tick);
    if (commandsConfig.func.enabled) commands::registerFuncCommand(commandsConfig.func);
    if (commandsConfig.self.enabled) commands::registerSelfCommand(commandsConfig.self);
    if (commandsConfig.hsa.enabled) commands::registerHsaCommand(commandsConfig.hsa);
    if (commandsConfig.counter.enabled) commands::registerCounterCommand(commandsConfig.counter);
    if (commandsConfig.prof.enabled) commands::registerProfCommand(commandsConfig.prof);
    if (commandsConfig.slime.enabled) commands::registerSlimeCommand(commandsConfig.slime);
    if (commandsConfig.village.enabled) commands::registerVillageCommand(commandsConfig.village);
    if (commandsConfig.rotate.enabled) commands::registerRotateCommand(commandsConfig.rotate);
    if (commandsConfig.data.enabled) commands::registerDataCommand(commandsConfig.data);
    if (commandsConfig.cfhud.enabled) commands::registerCfhudCommand(commandsConfig.cfhud);
    if (commandsConfig.log.enabled) commands::registerLogCommand(commandsConfig.log);
    if (commandsConfig.calculate.enabled) commands::registerCalculateCommand(commandsConfig.calculate);
    if (commandsConfig.minerule.enabled) commands::registerMineruleCommand(commandsConfig.minerule);
    if (commandsConfig.freecamera.enabled) commands::registerFreeCameraCommand(commandsConfig.freecamera);
    if (commandsConfig.noclip.enabled) commands::registerNoclipCommand(commandsConfig.noclip);
    if (commandsConfig.locate.enabled) commands::registerLocateCommand(commandsConfig.locate);

    functions::ShortcutsManager::getInstance().loadData();
    functions::ShortcutsManager::getInstance().registerShortcutsCommand();
}

// void CoralFans::unhook() {
//     functions::FreeCameraManager::freecameraHook(false);
//     functions::autoItemHook(false);
//     functions::autoTotemHook(false);
//     functions::hookAutoTool(false);
//     functions::registerContainerReader();
//     functions::forceOpenHook(false);
//     functions::forcePlaceHook(0);
//     functions::safeExplodeHook(false);
//     functions::fastDropHook(false);
//     functions::noPickUpHook(false);
//     functions::portalDisabledHook(false);
//     functions::FuncDropNoCostManager::droppernocostHook(false);
//     functions::HopperCounterManager::getInstance().setEnabled(false);
//     functions::locate::DuplicatableManager::hook(false);
//     functions::bedrockDropHook(false);
//     functions::mbDropHook(false);
//     functions::portalSandFarmHook(false);
//     functions::portalSpawnHook(false);
//     functions::restoreAncillaryBrokenHook(false);
//     functions::populationCapHook(false);
//     functions::noclipHook(false);
//     functions::hookTick(false);
//     functions::CFVillageManager::hookVillage(false);
// }

void CoralFans::removeRuntimeData() {
    functions::FreeCameraManager::getInstance().FreeCamList.clear();
    functions::HopperCounterManager::getInstance().clearAllData();
    functions::HsaManager::getInstance().setHsaShow(false);
    functions::HsaManager::getInstance().setStructureShow(false);
    functions::locate::DuplicatableManager::getInstance().clear();
    functions::PopulationCapManager::getInstance().clear();
    functions::ShortcutsManager::getInstance().clear();
    functions::SlimeManager::getInstance().setShow(false);
    functions::CFVillageManager::getInstance().clear();
    my_schedule::MySchedule::getSchedule().clear();
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

    getEventListeners().emplace(ll::event::EventBus::getInstance()
                                    .emplaceListener<ll::event::command::ServerCommandRegisterEvent>([this](auto&&) {
#ifdef LL_PLAT_S
                                        auto configDbPath = getSelf().getDataDir() / "config";
#endif
#ifdef LL_PLAT_C
                                        auto dataPath     = getSelf().getWorldDataDir();
                                        auto configDbPath = dataPath.has_value() ? dataPath.value() / "config"
                                                                                 : getSelf().getDataDir() / "config";
#endif
                                        getConfigDb() = std::make_unique<ll::data::KeyValueDB>(configDbPath);

                                        // load GeometryGroup
                                        getGeometryGroup() = bsci::GeometryGroup::createDefault();
                                        setupCommands();
                                    }));
    getEventListeners().emplace(
        ll::event::EventBus::getInstance().emplaceListener<ll::event::server::ServerStoppingEvent>([this](auto&&) {
            removeRuntimeData();
            getConfigDb()      = nullptr;
            getGeometryGroup() = nullptr;
        })
    );
    functions::ShortcutsManager::getInstance().registerShortcutsListener();
    return true;
}

bool CoralFans::enable() {
    const auto& logger = getSelf().getLogger();
    logger.debug("Enabling...");

    return true;
}

bool CoralFans::disable() {
    getSelf().getLogger().debug("Disabling...");
    // unhook();
    removeRuntimeData();
    return true;
}

} // namespace coral_fans

LL_REGISTER_MOD(coral_fans::CoralFans, coral_fans::CoralFans::getInstance());
