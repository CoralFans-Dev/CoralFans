#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/Level.h"

namespace {

bool autoItem(Player& player, ItemStack const& item) {
    auto& inv  = player.getInventory();
    int   size = inv.getContainerSize();
    for (int i = 0; i < size; ++i) {
        if (i == player.getSelectedItemSlot()) continue;
        auto& itemi = inv.getItem(i);
        if (itemi.mCount != 0) {
            if (coral_fans::utils::removeMinecraftPrefix(itemi.getTypeName()).ends_with("_shulker_box")) {
                auto tag = itemi.save();
                auto rst = coral_fans::utils::getItemFromShulkerBox(std::move(tag), item, true, 1);
                if (rst) {
                    inv.setItem(i, ItemStack::fromTag(rst->first));
                    player.setSelectedItem(ItemStack::fromTag(rst->second));
                    player.refreshInventory();
                    return true;
                }
            }
            if (itemi.matchesItem(item) && itemi.mCount > 1) {
                coral_fans::utils::swapItemInContainer(&player, i, player.getSelectedItemSlot());
                player.refreshInventory();
                return true;
            }
        }
    }
    return false;
}

} // namespace

namespace coral_fans::functions {

void registerAutoItemListener() {
    auto& mod = coral_fans::mod();
    // PlayerUseItemEvent
    ll::event::ListenerPtr useListener;
    useListener = mod.getEventBus().emplaceListener<ll::event::PlayerUseItemEvent>([&](ll::event::PlayerUseItemEvent& ev
                                                                                   ) {
        auto& item   = ev.item();
        auto& player = ev.self();
        if (item.mCount == 1 && !item.isBlock()
            && coral_fans::mod().getConfigDb()->get("functions.players." + player.getUuid().asString() + ".autoitem")
                   == "true"
            && ::autoItem(player, item))
            ev.cancel();
    });
    mod.getEventListeners().emplace(useListener);
    // PlayerPlaceBlockEvent
    ll::event::ListenerPtr placedListener;
    placedListener =
        mod.getEventBus().emplaceListener<ll::event::PlayerPlacingBlockEvent>([&](ll::event::PlayerPlacingBlockEvent& ev
                                                                              ) {
            auto& player = ev.self();
            auto& item   = player.getSelectedItem();
            if (item.mCount == 1
                && coral_fans::mod().getConfigDb()->get(
                       "functions.players." + player.getUuid().asString() + ".autoitem"
                   ) == "true"
                && ::autoItem(player, item))
                ev.cancel();
        });
    mod.getEventListeners().emplace(placedListener);
}

} // namespace coral_fans::functions