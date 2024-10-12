#include "coral_fans/base/Mod.h"

namespace coral_fans {

void CoralFansMod::tick() {
    this->getScheduler().tick();
    this->getHopperCounterManager().tick(); // light 1
    this->getHsaManager().tick();           // heavy 80
    this->getSlimeManager().tick();         // heavy 80
    this->getVillageManager().lightTick();  // light 20
    this->getVillageManager().heavyTick();  // heavy 40
    this->mHudHelper.tick();                // light 20
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans