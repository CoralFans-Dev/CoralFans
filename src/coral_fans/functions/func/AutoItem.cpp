#include "coral_fans/CoralFans.h"


#include "ll/api/memory/Hook.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/IntTag.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/item/ShulkerBoxBlockItem.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include <optional>
#include <string>
#include <vector>


namespace coral_fans::functions {

std::optional<ItemStack> handleBundle(
    ::std::unique_ptr<Inventory>&   inv,
    int                             selectedSlot,
    const std::string&              targetName,
    int                             handlingSlot,
    std::unique_ptr<::CompoundTag>& originTag,
    CompoundTag::TagMap&            bundleTag
) {
    auto& selectedItem     = inv->getItem(selectedSlot);
    auto  selectedItemName = selectedItem.getTypeName();
    auto  it1              = bundleTag.find("tag");
    if (it1 == bundleTag.end()) return std::nullopt;
    auto& temCompoundTag = (*it1).second.get<CompoundTag>().mTags;
    auto  weightIt       = temCompoundTag.find("bundle_weight");
    if (weightIt == temCompoundTag.end()) return std::nullopt;
    auto& weight = weightIt->second.get<IntTag>();
    auto  it2    = temCompoundTag.find("storage_item_component_content");
    if (it2 == temCompoundTag.end()) return std::nullopt;
    auto&  list = it2->second.get<ListTag>();
    size_t size = list.size();
    for (size_t i = 0; i < size; i++) {
        auto& itemTag = list[i].get<CompoundTag>();
        auto  nameIt  = itemTag.mTags.find("Name");
        if (nameIt == itemTag.mTags.end()) continue;
        auto name = nameIt->second.get<StringTag>();
        if (name == targetName) {
            auto newItem = ItemStack::fromTag(itemTag);
            if (selectedItem == ItemStack::EMPTY_ITEM()) {
                list.erase(list.begin() + i);
                while (i < size - 1) {
                    auto& temCompoundTag2 = list[i].get<CompoundTag>().mTags;
                    auto  countIt         = temCompoundTag2.find("Count");
                    if (countIt != temCompoundTag2.end() && countIt->second.get<ByteTag>().data == 0) {
                        auto emptyTag = ItemStack::EMPTY_ITEM().save(*SaveContextFactory::createCloneSaveContext());
                        emptyTag->mTags["Slot"] = ByteTag(i);
                        list.insert(list.begin() + i, CompoundTagVariant(emptyTag->mTags));
                        break;
                    } else {
                        temCompoundTag2["Slot"] = ByteTag(i);
                    }
                    i++;
                }
                weight.data -= newItem.mCount * 64 / newItem.mItem->mMaxStackSize;
                inv->setItem(handlingSlot, ItemStack::fromTag(*originTag));
                return std::move(newItem);
            } else if (weight.data - newItem.mCount * 64 / newItem.mItem->mMaxStackSize
                               + selectedItem.mCount * 64 / selectedItem.mItem->mMaxStackSize
                           > 68
                       || selectedItemName.ends_with("_shulker_box") || selectedItemName.ends_with("bundle")) {
                int firstEmptySlot = inv->getFirstEmptySlot();
                if (firstEmptySlot == -1) return std::nullopt;
                list.erase(list.begin() + i);
                while (i < size - 1) {
                    auto& temCompoundTag2 = list[i].get<CompoundTag>().mTags;
                    auto  countIt         = temCompoundTag2.find("Count");
                    if (countIt != temCompoundTag2.end() && countIt->second.get<ByteTag>().data == 0) {
                        auto emptyTag = ItemStack::EMPTY_ITEM().save(*SaveContextFactory::createCloneSaveContext());
                        emptyTag->mTags["Slot"] = ByteTag(i);
                        list.insert(list.begin() + i, CompoundTagVariant(emptyTag->mTags));
                        break;
                    } else {
                        temCompoundTag2["Slot"] = ByteTag(i);
                    }
                    i++;
                }
                weight.data -= newItem.mCount * 64 / newItem.mItem->mMaxStackSize;
                inv->setItem(handlingSlot, ItemStack::fromTag(*originTag));
                inv->swapSlots(firstEmptySlot, selectedSlot);
                return std::move(newItem);
            } else if (auto slotIt = itemTag.mTags.find("Slot"); slotIt != itemTag.mTags.end()) {
                auto selectedTag           = selectedItem.save(*SaveContextFactory::createCloneSaveContext());
                selectedTag->mTags["Slot"] = slotIt->second.get<ByteTag>();
                list[i]                    = CompoundTagVariant(selectedTag->mTags);
                weight.data                = weight.data - newItem.mCount * 64 / newItem.mItem->mMaxStackSize
                            + selectedItem.mCount * 64 / selectedItem.mItem->mMaxStackSize;
                inv->setItem(handlingSlot, ItemStack::fromTag(*originTag));
                return std::move(newItem);
            }
        } else if (name == "") return std::nullopt;
    }
    return std::nullopt;
}

std::optional<ItemStack>
autoItemByItemName(::std::unique_ptr<Inventory>& inv, int selectedSlot, const std::string& targetName) {
    auto&            selectedItem     = inv->getItem(selectedSlot);
    auto             selectedItemName = selectedItem.getTypeName();
    int              size             = static_cast<int>(inv->getContainerSize());
    std::vector<int> containerSlots;
    if (targetName == selectedItemName) return std::nullopt;
    for (int i = 0; i < size; i++) {
        const ItemStack& item     = inv->getItem(i);
        auto             itemName = item.getTypeName();
        if (itemName == targetName) {
            std::optional<ItemStack> result = item;
            inv->setItem(i, selectedItem);
            return result;
        }
        if (itemName.ends_with("_shulker_box") || itemName.ends_with("bundle")) {
            containerSlots.push_back(i);
        }
    }
    for (auto slot : containerSlots) {
        const ItemStack& item     = inv->getItem(slot);
        auto             itemName = item.getTypeName();
        auto             tag      = item.save(*SaveContextFactory::createCloneSaveContext());
        if (!tag) continue;
        if (itemName.ends_with("_shulker_box")) {
            auto it1 = tag->mTags.find("tag");
            if (it1 == tag->mTags.end()) continue;
            auto& temCompoundTag = (*it1).second.get<CompoundTag>().mTags;
            auto  it2            = temCompoundTag.find("Items");
            if (it2 == temCompoundTag.end()) continue;
            auto& list = it2->second.get<ListTag>();
            size       = static_cast<int>(list.size());
            for (int _i = 0; _i < size; _i++) {
                auto& itemTag = list[_i].get<CompoundTag>();
                if (auto nameIt = itemTag.mTags.find("Name"); nameIt != itemTag.mTags.end()) {
                    auto name = nameIt->second.get<StringTag>();
                    if (name == targetName) {
                        if (selectedItem == ItemStack::EMPTY_ITEM()) {
                            auto newItemTag = itemTag;
                            list.erase(list.begin() + _i);
                            inv->setItem(slot, ItemStack::fromTag(*tag));
                            return ItemStack::fromTag(newItemTag);
                        } else if (selectedItemName.ends_with("_shulker_box")) {
                            int firstEmptySlot = inv->getFirstEmptySlot();
                            if (firstEmptySlot == -1) break;
                            auto newItemTag = itemTag;
                            list.erase(list.begin() + _i);
                            inv->setItem(slot, ItemStack::fromTag(*tag));
                            inv->swapSlots(firstEmptySlot, selectedSlot);
                            return ItemStack::fromTag(newItemTag);
                        } else if (auto slotIt = itemTag.mTags.find("Slot"); slotIt != itemTag.mTags.end()) {
                            auto selectedTag = selectedItem.save(*SaveContextFactory::createCloneSaveContext());
                            selectedTag->mTags["Slot"] = slotIt->second.get<ByteTag>();
                            auto newItemTag            = itemTag;
                            list[_i]                   = CompoundTagVariant(selectedTag->mTags);
                            inv->setItem(slot, ItemStack::fromTag(*tag));
                            return ItemStack::fromTag(newItemTag);
                        }
                    } else if (name.ends_with("bundle"))
                        return handleBundle(inv, selectedSlot, targetName, slot, tag, itemTag.mTags);
                }
            }
        } else return handleBundle(inv, selectedSlot, targetName, slot, tag, tag->mTags);
    }
    return std::nullopt;
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansAutoItemHook,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$useItem,
    void,
    ::ItemStackBase& item,
    ::ItemUseMethod  itemUseMethod,
    bool             consumeItem
) {
    if ((CoralFans::getInstance().getConfigDb()->get(
             std::format("functions.players.{}.autoitem", this->getUuid().asString())
         )
         == "false")
        || this->isCreative())
        return origin(item, itemUseMethod, consumeItem);
    std::string name = item.getTypeName();
    if (!this->mInventory->mInventory) return;
    auto& inv          = this->mInventory->mInventory;
    int   selectedSlot = this->getSelectedItemSlot();
    if (name.ends_with("_shulker_box")) {
        int size = inv->getContainerSize();
        for (int i = 0; i < size; i++) {
            if (i == selectedSlot) continue;
            const ItemStack& itemi = inv->getItem(i);
            if (item.matchesItem(itemi)) return inv->setItem(i, ItemStack::EMPTY_ITEM());
        }
        origin(item, itemUseMethod, consumeItem);
    } else {
        origin(item, itemUseMethod, consumeItem);
        if (item == ItemStack::EMPTY_ITEM()) {
            auto selectedItem = inv->getItem(selectedSlot);
            inv->setItem(selectedSlot, ItemStack::EMPTY_ITEM());
            auto res = autoItemByItemName(inv, selectedSlot, name);
            if (res.has_value()) item = std::move(res.value());
            inv->setItem(selectedSlot, selectedItem);
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansAutoItemHook2,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::handleBlockPickRequestOnServer,
    void,
    ::BlockPos const& position,
    bool              withData
) {
    if ((CoralFans::getInstance().getConfigDb()->get(
             std::format("functions.players.{}.autoitem", this->getUuid().asString())
         )
         == "false")
        || this->isCreative())
        return origin(position, withData);
    auto& region     = this->getDimensionBlockSource();
    auto  targetItem = region.getBlock(position).asItemInstance(region, position, withData);
    auto  targetName = targetItem.getTypeName();
    if (!this->mInventory || !this->mInventory->mInventory) return;
    auto& inv          = this->mInventory->mInventory;
    int   selectedSlot = this->getSelectedItemSlot();
    if (targetName.ends_with("_shulker_box")) {
        int size = inv->getContainerSize();
        for (int i = 0; i < size; i++) {
            if (i == selectedSlot) continue;
            const ItemStack& item = inv->getItem(i);
            if (targetItem.matchesItem(item)) {
                inv->swapSlots(selectedSlot, i);
                return;
            }
        }
    } else {
        auto res = autoItemByItemName(inv, selectedSlot, targetName);
        if (res.has_value()) {
            inv->setItem(selectedSlot, res.value());
            this->refreshInventory();
        }
    }
}

void autoItemHook(bool bl) {
    if (bl) {
        CoralFansAutoItemHook::hook();
        CoralFansAutoItemHook2::hook();
    } else {
        CoralFansAutoItemHook::unhook();
        CoralFansAutoItemHook2::unhook();
    }
}
} // namespace coral_fans::functions