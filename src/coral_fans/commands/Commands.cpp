#include "Commands.h"

namespace coral_fans::commands {
std::optional<Player*> tryGetPlayer(CommandOrigin const& origin) {
    auto oriType = origin.getOriginType();
    if (oriType == CommandOriginType::DedicatedServer) return nullptr;
    if (oriType != CommandOriginType::Player) return std::nullopt;
    auto* entity = origin.getEntity();
    if (entity == nullptr || !entity->isType(ActorType::Player)) return std::nullopt;
    return static_cast<Player*>(entity);
}
} // namespace coral_fans::commands