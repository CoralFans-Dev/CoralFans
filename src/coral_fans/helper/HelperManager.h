#pragma once

namespace coral_fans::help {
class HelperManager {

public:
    static HelperManager& getInstance() {
        static HelperManager instance;
        return instance;
    }
};
} // namespace coral_fans::help