#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/SaveContextFactory.h"


namespace coral_fans::functions {

LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersAutoTotemHook,
    ll::memory::HookPriority::Normal,
    Player,
    &Player::$consumeTotem,
    bool
) {
    if (coral_fans::mod().getConfigDb()->get("functions.players." + this->getUuid().asString() + ".autototem")
        == "true") {
        auto& inv  = this->getInventory();
        int   size = inv.getContainerSize();
        for (int i = 0; i < size; i++) {
            const ItemStack& item = inv.getItem(i);
            if (item.getTypeName().ends_with("_shulker_box")) {
                auto tag = item.save(*SaveContextFactory::createCloneSaveContext());
                if (!tag->contains("tag")) continue;
                auto list  = (*tag)["tag"]["Items"].get<ListTag>();
                int  _size = list.size();
                for (int _i = 0; _i < _size; _i++) {
                    auto itemTag = list.getCompound(_i);
                    if ((*itemTag)["Name"].get<StringTag>() == "minecraft:totem_of_undying") {
                        list.erase(list.begin() + _i);
                        (*tag)["tag"]["Items"] = list;
                        inv.setItem(i, ItemStack::fromTag(*tag));
                        this->refreshInventory();
                        return false;
                    }
                }
            }
            if (i == this->getSelectedItemSlot()) continue;
            if (item.getTypeName() == "minecraft:totem_of_undying") {
                inv.setItem(i, ItemStack::EMPTY_ITEM());
                this->refreshInventory();
                return false;
            }
        }
    }
    return origin();
}

void autoTotemHook(bool bl) { bl ? CoralFansTweakersAutoTotemHook::hook() : CoralFansTweakersAutoTotemHook::unhook(); }

} // namespace coral_fans::functions