#include "coral_fans/functions/func/Droppernocost.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/Container.h"
#include "mc/world/item/FertilizerItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/block/actor/DispenserBlockActor.h"


namespace coral_fans::functions::droppernocost {
int  _slot, _count;
bool mutex = false, mutex2 = true;

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
    if (mutex) {
        origin(slot, count);
        mutex = false;
    } else {
        _slot = slot, _count = count;
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
    mutex = true;
    this->removeItem(_slot, _count);
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
    mutex2 = false;
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
    if (mutex2) origin(slot, item);
    else mutex2 = true;
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
} // namespace coral_fans::functions::droppernocost