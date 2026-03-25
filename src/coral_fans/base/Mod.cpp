#include "coral_fans/base/Mod.h"
#include "coral_fans/base/MySchedule.h"
#include "coral_fans/functions/locate/DuplicatableManager.h"

namespace coral_fans {

void CoralFansMod::tick(const Tick& currentTick) {
    this->getHopperCounterManager().tick();      // light 1
    this->getHsaManager().tick();                // heavy 1200
    this->getSlimeManager().tick();              // heavy 80
    this->getVillageManager().tick(currentTick); // light 10
    this->mHudHelper.tick();                     // light 20
    my_schedule::MySchedule::getSchedule().update();
    functions::locate::DuplicatableManager::getInstance().tick();
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans