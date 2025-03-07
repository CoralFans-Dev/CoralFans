#include "coral_fans/functions/func/Droppernocost.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/Container.h"
#include "mc/world/item/FertilizerItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/block/actor/DispenserBlockActor.h"


namespace coral_fans::functions {

// droppernocost
LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersDropperNoCostHook,
    ll::memory::HookPriority::Normal,
    Container,
    &Container::$removeItem,
    void,
    int slot,
    int count
) {
    if (FuncDropNoCostManager::getInstance().mutex) {
        origin(slot, count);
        FuncDropNoCostManager::getInstance().mutex = false;
    } else {
        FuncDropNoCostManager::getInstance()._slot = slot, FuncDropNoCostManager::getInstance()._count = count;
    }
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersDropperNoCostHook2,
    ll::memory::HookPriority::Normal,
    Container,
    &Container::$addItemToFirstEmptySlot,
    bool,
    ::ItemStack const& item
) {
    FuncDropNoCostManager::getInstance().mutex = true;
    this->removeItem(FuncDropNoCostManager::getInstance()._slot, FuncDropNoCostManager::getInstance()._count);
    return origin(item);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersDropperNoCostHook3,
    ll::memory::HookPriority::Normal,
    FertilizerItem,
    &FertilizerItem ::$dispense,
    bool,
    ::BlockSource& region,
    ::Container&   container,
    int            slot,
    ::Vec3 const&  pos,
    uchar          face
) {
    FuncDropNoCostManager::getInstance().mutex2 = false;
    return origin(region, container, slot, pos, face);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansTweakersDropperNoCostHook4,
    ll::memory::HookPriority::Normal,
    DispenserBlockActor,
    &DispenserBlockActor ::$setItem,
    void,
    int                slot,
    ::ItemStack const& item
) {
    if (FuncDropNoCostManager::getInstance().mutex2) origin(slot, item);
    else FuncDropNoCostManager::getInstance().mutex2 = true;
}

void droppernocostHook(bool bl) {
    if (bl) {
        CoralFansTweakersDropperNoCostHook::hook();
        CoralFansTweakersDropperNoCostHook2::hook();
        CoralFansTweakersDropperNoCostHook3::hook();
        CoralFansTweakersDropperNoCostHook4::hook();
    } else {
        CoralFansTweakersDropperNoCostHook::unhook();
        CoralFansTweakersDropperNoCostHook2::unhook();
        CoralFansTweakersDropperNoCostHook3::unhook();
        CoralFansTweakersDropperNoCostHook4::unhook();
    }
}
} // namespace coral_fans::functions