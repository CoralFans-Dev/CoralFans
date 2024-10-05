#pragma once

#include "ll/api/base/StdInt.h"
#include <string>
#include <utility>
#include <vector>

namespace coral_fans::functions {

class HudHelper {
public:
    enum HudType : unsigned long { mspt, base, redstone, village, hopper };
    static std::vector<std::pair<std::string, uint64>> HudTypeVec;

public:
    void tick();
};

} // namespace coral_fans::functions