#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/registry/ItemStack.h"
#include <utility>

namespace coral_fans::functions {

LL_AUTO_TYPE_INSTANCE_HOOK(
    CoralFansTweakersAutoTotemHook,
    ll::memory::HookPriority::Normal,
    Player,
    "?consumeTotem@Player@@UEAA_NXZ",
    bool
) {
    if (coral_fans::mod().getConfigDb()->get("functions.players." + this->getUuid().asString() + ".autototem")
        == "true") {
        auto& inv  = this->getInventory();
        int   size = inv.getContainerSize();
        for (int i = 0; i < size; ++i) {
            auto& item = inv.getItem(i);
            if (item.mCount != 0) {
                if (utils::removeMinecraftPrefix(item.getTypeName()).ends_with("_shulker_box")) {
                    auto tag = item.save();
                    auto rst = utils::getItemFromShulkerBox(std::move(tag), "totem_of_undying");
                    if (rst) {
                        inv.setItem(i, ItemStack::fromTag(rst->first));
                        this->refreshInventory();
                        return false;
                    }
                }
                if (i == this->getSelectedItemSlot()) continue;
                if (utils::removeMinecraftPrefix(item.getTypeName()) == "totem_of_undying") {
                    inv.setItem(i, ItemStack::EMPTY_ITEM);
                    this->refreshInventory();
                    return false;
                }
            }
        }
    }
    return origin();
}

} // namespace coral_fans::functions