#pragma once

#include "Config.h"
#include "bsci/GeometryGroup.h"
#include "ll/api/data/KeyValueDB.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/mod/NativeMod.h"


namespace coral_fans {

class CoralFans {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    CoralFans();
    ~CoralFans();

    static CoralFans& getInstance();

    [[nodiscard]] config::Config& getConfig();

    [[nodiscard]] std::unique_ptr<ll::data::KeyValueDB>& getConfigDb();

    [[nodiscard]] std::unique_ptr<bsci::GeometryGroup>& getGeometryGroup();

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    [[nodiscard]] std::set<ll::event::ListenerPtr>& getEventListeners();

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace coral_fans
