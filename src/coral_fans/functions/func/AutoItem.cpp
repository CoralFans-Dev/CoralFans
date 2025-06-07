#include "coral_fans/base/Mod.h"
#include "ll/api/memory/Hook.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/Container.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/item/ShulkerBoxBlockItem.h"
#include <string>


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(
    CoralFansAutoItemHook,
    ll::memory::HookPriority::Normal,
    Player,
    &Player::$useItem,
    void,
    ::ItemStackBase& item,
    ::ItemUseMethod  itemUseMethod,
    bool             consumeItem
) {
    if ((coral_fans::mod().getConfigDb()->get(std::format("functions.players.{}.autoitem", this->getUuid().asString()))
         == "false"))
        return origin(item, itemUseMethod, consumeItem);
    std::string name = item.getTypeName();
    if (name.ends_with("_shulker_box")) {
        Container& inv          = *this->mInventory->mInventory;
        int        size         = inv.getContainerSize();
        int        selectedSlot = this->getSelectedItemSlot();
        for (int i = 0; i < size; i++) {
            if (i == selectedSlot) continue;
            const ItemStack& itemi = inv.getItem(i);
            if (item.matchesItem(itemi)) {
                inv.setItem(i, ItemStack::EMPTY_ITEM());
                return;
            }
        }
        origin(item, itemUseMethod, consumeItem);
    } else {
        origin(item, itemUseMethod, consumeItem);
        if (item == ItemStack::EMPTY_ITEM()) {
            Container& inv          = *this->mInventory->mInventory;
            int        size         = inv.getContainerSize();
            int        selectedSlot = this->getSelectedItemSlot();
            for (int i = 0; i < size; i++) {
                if (i == selectedSlot) continue;
                const ItemStack& itemi = inv.getItem(i);
                if (itemi.getTypeName() == name) {
                    item = itemi;
                    inv.setItem(i, ItemStack::EMPTY_ITEM());
                    return;
                }
                if (itemi.getTypeName().ends_with("_shulker_box")) {
                    auto tag = itemi.save(*SaveContextFactory::createCloneSaveContext());
                    if (!tag || !tag->contains("tag") || !(*tag)["tag"].contains("Items")) continue;
                    auto list  = (*tag)["tag"]["Items"].get<ListTag>();
                    int  _size = list.size();
                    for (int _i = 0; _i < _size; _i++) {
                        auto itemTag = list.getCompound(_i);
                        if ((*itemTag)["Name"].get<StringTag>() == name) {
                            item = ItemStack::fromTag(*itemTag);
                            list.erase(list.begin() + _i);
                            (*tag)["tag"]["Items"] = list;
                            inv.setItem(i, ItemStack::fromTag(*tag));
                            return;
                        }
                    }
                }
            }
        }
    }
}

void autoItemHook(bool bl) { bl ? CoralFansAutoItemHook::hook() : CoralFansAutoItemHook::unhook(); }
} // namespace coral_fans::functions