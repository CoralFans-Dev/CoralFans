#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/Level.h"

namespace coral_fans::functions {

void registerAutoItemListener() {
    ll::event::ListenerPtr evListener;
    auto&                  mod = coral_fans::mod();
    evListener = mod.getEventBus().emplaceListener<ll::event::PlayerUseItemEvent>([&](ll::event::PlayerUseItemEvent& ev
                                                                                  ) {
        auto& item   = ev.item();
        auto& player = ev.self();
        if (item.mCount == 1) {
            if (coral_fans::mod().getConfigDb()->get("functions.players." + player.getUuid().asString() + ".autoitem")
                == "true") {
                auto& inv  = player.getInventory();
                int   size = inv.getContainerSize();
                for (int i = 0; i < size; ++i) {
                    if (i == player.getSelectedItemSlot()) continue;
                    auto& itemi = inv.getItem(i);
                    if (itemi.mCount != 0) {
                        if (utils::removeMinecraftPrefix(itemi.getTypeName()).ends_with("_shulker_box")) {
                            auto tag = itemi.save();
                            auto rst = utils::getItemFromShulkerBox(std::move(tag), item, true, 1);
                            if (rst) {
                                inv.setItem(i, ItemStack::fromTag(rst->first));
                                player.setSelectedItem(ItemStack::fromTag(rst->second));
                                player.refreshInventory();
                                ev.cancel();
                                return;
                            }
                        }
                        if (itemi.matchesItem(item) && itemi.mCount > 1) {
                            utils::swapItemInContainer(&player, i, player.getSelectedItemSlot());
                            player.refreshInventory();
                            ev.cancel();
                            break;
                        }
                    }
                }
            }
        }
    });
    mod.getEventListeners().emplace(evListener);
}

} // namespace coral_fans::functions