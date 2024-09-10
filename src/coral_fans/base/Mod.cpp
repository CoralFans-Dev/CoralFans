#include "coral_fans/base/Mod.h"

namespace coral_fans {

void CoralFansMod::lightTick() {
    this->getHopperCounterManager().tick();
    return;
}

void CoralFansMod::heavyTick() {
    this->getHsaManager().tick();
    return;
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans