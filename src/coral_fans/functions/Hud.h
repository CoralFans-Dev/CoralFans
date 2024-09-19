#pragma once

namespace coral_fans::functions {

class HudHelper {
public:
    enum HudType : unsigned long { mspt, base, redstone, village, hopper };

public:
    void tick();
};

} // namespace coral_fans::functions