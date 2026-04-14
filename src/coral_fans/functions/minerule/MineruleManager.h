#pragma once

namespace coral_fans::functions {
class MineruleManager {
public:
    static MineruleManager& getInstance() {
        static MineruleManager instance;
        return instance;
    }

public:
    void bedrockDropHook(bool);
    void mbDropHook(bool);
    void portalSandFarmHook(bool);
    void portalSpawnHook(bool);
    void restoreAncillaryBrokenHook(bool);
    void populationCapHook(bool);
};
} // namespace coral_fans::functions