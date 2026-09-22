#pragma once

#include "bsci/GeometryGroup.h"

#include "mc/world/level/BlockPos.h"

#include <vector>

namespace coral_fans::functions::locate {

class PortalManager {
public:
    struct PortalInfo {
        int      dimId;
        BlockPos pos;

        bool operator==(PortalInfo const&) const = default;
    };

private:
    bool                                mShow       = false;
    int                                 tickCounter = 0;
    std::vector<bsci::GeometryGroup::GeoId> mGeoIds;
    std::vector<PortalInfo>             mDrawnPortals;

public:
    void setShow(bool);
    bool getShow() const;
    void clear();

    void tick();

    static std::vector<PortalInfo> getSortedPortals();

private:
    void draw();
    void remove();

private:
    PortalManager() = default;

public:
    [[nodiscard]] static PortalManager& getInstance() {
        static PortalManager instance;
        return instance;
    }

    PortalManager(const PortalManager&)            = delete;
    PortalManager& operator=(const PortalManager&) = delete;
};

} // namespace coral_fans::functions::locate
