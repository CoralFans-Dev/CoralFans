#include "coral_fans/base/Mod.h"
#include "coral_fans/base/MySchedule.h"
#include "coral_fans/functions/func/FuncManager.h"
#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/functions/hud/Hud.h"
#include "coral_fans/functions/locate/DuplicatableManager.h"
#include "coral_fans/functions/slime/Slime.h"
#include "coral_fans/functions/village/Village.h"


namespace coral_fans {

void CoralFansMod::tick(const Tick& currentTick) {
    functions::HopperCounterManager::getInstance().tick();
    functions::HsaManager::getInstance().tick();                  // heavy 60
    functions::SlimeManager::getInstance().tick();                // heavy 60
    functions::CFVillageManager::getInstance().tick(currentTick); // light 10
    functions::HudHelper::getInstance().tick();                   // light 20
    my_schedule::MySchedule::getSchedule().update();
    functions::locate::DuplicatableManager::getInstance().tick();
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans