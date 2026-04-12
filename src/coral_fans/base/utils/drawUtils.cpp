#include "bsci/GeometryGroup.h"


namespace coral_fans::utils {

inline std::pair<Vec3, Vec3> branchlessONB(Vec3 const& n) {
    float const sign = std::copysign(1.0f, n.z);
    float const a    = -1.0f / (sign + n.z);
    float const b    = n.x * n.y * a;
    return {
        {1.0f + sign * n.x * n.x * a, sign * b,             -sign * n.x},
        {b,                           sign + n.y * n.y * a, -n.y       }
    };
}

bsci::GeometryGroup::GeoId drawCylinder(
    bsci::GeometryGroup& geo,
    DimensionType        dim,
    Vec3 const&          topCenter,
    Vec3 const&          bottomCenter,
    float                topRadius,
    float                bottomRadius,
    mce::Color const&    color
) {

    size_t const points = 10;
    auto const [t, b]   = branchlessONB((topCenter - bottomCenter).normalize());
    auto const delta    = std::numbers::pi * 2 / (double)points;

    std::vector<bsci::GeometryGroup::GeoId> ids;
    ids.reserve(points);
    Vec3 lastTopPos    = t * topRadius;
    Vec3 lastBottomPos = b * bottomRadius;
    for (size_t i{1}; i <= points; i++) {
        double theta     = (double)i * delta;
        Vec3   topPos    = t * (topRadius * std::cos(theta)) + b * bottomRadius * std::sin(theta);
        Vec3   bottomPos = t * (bottomRadius * std::cos(theta)) + b * bottomRadius * std::sin(theta);
        ids.emplace_back(geo.line(dim, topCenter + lastTopPos, topCenter + topPos, color));
        ids.emplace_back(geo.line(dim, topCenter + topPos, bottomCenter + bottomPos, color));
        ids.emplace_back(geo.line(dim, bottomCenter + lastBottomPos, bottomCenter + bottomPos, color));
        lastTopPos    = topPos;
        lastBottomPos = bottomPos;
    }
    return geo.merge(ids);
}
} // namespace coral_fans::utils