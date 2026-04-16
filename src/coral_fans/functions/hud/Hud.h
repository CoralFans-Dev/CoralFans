#pragma once

#include "ll/api/base/StdInt.h"
#include <string>
#include <utility>
#include <vector>

namespace coral_fans::functions {

class HudHelper {
public:
    enum HudType : unsigned long { mspt, base, redstone, village, hopper, block, container };
    static std::vector<std::pair<std::string, uint64>> HudTypeVec;

public:
    void tick();

private:
    HudHelper() = default;

public:
    [[nodiscard]] static HudHelper& getInstance() {
        static HudHelper instance;
        return instance;
    }
};

} // namespace coral_fans::functions