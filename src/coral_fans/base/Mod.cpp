#include "coral_fans/base/Mod.h"

namespace coral_fans {

void CoralFansMod::tick() {
    this->getHopperCounterManager().tick();
    this->getHsaManager().tick();
    this->getSlimeManager().tick();
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans