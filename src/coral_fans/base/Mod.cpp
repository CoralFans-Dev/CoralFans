#include "coral_fans/base/Mod.h"
#include "coral_fans/base/MySchedule.h"

namespace coral_fans {

void CoralFansMod::tick() {
    this->getHopperCounterManager().tick(); // light 1
    this->getHsaManager().tick();           // heavy 80
    this->getSlimeManager().tick();         // heavy 80
    this->getVillageManager().lightTick();  // light 20
    this->getVillageManager().heavyTick();  // heavy 40
    this->mHudHelper.tick();                // light 20
    my_schedule::MySchedule::getSchedule().update();
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans