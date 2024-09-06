#include "coral_fans/base/Mod.h"

namespace coral_fans {

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans