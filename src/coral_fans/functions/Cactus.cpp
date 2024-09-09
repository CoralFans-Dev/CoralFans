#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/HopperCounter.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/level/Level.h"

namespace {

// anti shake from trapdoor-ll
struct UseOnAction {
    uint64_t gameTick = 0;
    BlockPos pos;

    bool operator==(const UseOnAction& rhs) const {
        if (pos != rhs.pos) return false;
        return (gameTick - rhs.gameTick) <= 3;
    }

    bool operator!=(const UseOnAction& rhs) const { return !(rhs == *this); }
};

std::unordered_map<std::string, UseOnAction>& getUseOnCache() {
    static std::unordered_map<std::string, UseOnAction> cache;
    return cache;
}

bool antiShake(const ServerPlayer& player, const BlockPos& pos) {
    const auto& playerUuid      = player.getUuid().asString();
    uint64_t    gt              = player.getLevel().getCurrentServerTick().t;
    auto        useOnAction     = UseOnAction{gt, pos};
    auto        lastUseOnAction = getUseOnCache()[playerUuid];
    if (useOnAction == lastUseOnAction) return false;
    getUseOnCache()[playerUuid] = useOnAction;
    return true;
}

} // namespace

namespace coral_fans::functions {

void registerCactusListener() {
    ll::event::ListenerPtr listener;
    listener = coral_fans::mod().getEventBus()->emplaceListener<ll::event::player::PlayerInteractBlockEvent>(
        [](ll::event::player::PlayerInteractBlockEvent& event) {
            if (utils::removeMinecraftPrefix(event.item().getTypeName()) != "cactus"
                || !::antiShake(event.self(), event.blockPos()))
                return;
            // hoppercounter
            if (event.block().has_value()
                && HopperCounterManager::HOPPER_COUNTER_MAP.find(
                       utils::removeMinecraftPrefix(event.block()->getTypeName())
                   ) != HopperCounterManager::HOPPER_COUNTER_MAP.end()) {
                event.self().sendMessage(
                    coral_fans::mod()
                        .getHopperCounterManager()
                        .getChannel(HopperCounterManager::HOPPER_COUNTER_MAP
                                        .find(utils::removeMinecraftPrefix(event.block()->getTypeName()))
                                        ->second)
                        .info()
                );
            }
        }
    );
    coral_fans::mod().getEventListeners().emplace(listener);
}

} // namespace coral_fans::functions