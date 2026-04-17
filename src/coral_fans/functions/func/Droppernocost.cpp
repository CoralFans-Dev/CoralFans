#include "FuncManager.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include "mc/world/Container.h"
#include "mc/world/item/FertilizerItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/block/actor/DispenserBlockActor.h"
#include <thread>


namespace coral_fans::functions {

// droppernocost
LL_TYPE_INSTANCE_HOOK(
    CoralFansDropperNoCostHook,
    ll::memory::HookPriority::Normal,
    Container,
    &Container::$removeItem,
    void,
    int slot,
    int count
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(slot, count);
#endif
    if (FuncDropNoCostManager::getInstance().mutex) {
        origin(slot, count);
        FuncDropNoCostManager::getInstance().mutex = false;
    } else {
        FuncDropNoCostManager::getInstance()._slot = slot, FuncDropNoCostManager::getInstance()._count = count;
    }
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropperNoCostHook2,
    ll::memory::HookPriority::Normal,
    Container,
    &Container::$addItemToFirstEmptySlot,
    bool,
    ::ItemStack const& item
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(item);
#endif
    FuncDropNoCostManager::getInstance().mutex = true;
    this->removeItem(FuncDropNoCostManager::getInstance()._slot, FuncDropNoCostManager::getInstance()._count);
    return origin(item);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropperNoCostHook3,
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
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region, container, slot, pos, face);
#endif
    FuncDropNoCostManager::getInstance().mutex2 = false;
    return origin(region, container, slot, pos, face);
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansDropperNoCostHook4,
    ll::memory::HookPriority::Normal,
    DispenserBlockActor,
    &DispenserBlockActor ::$setItem,
    void,
    int                slot,
    ::ItemStack const& item
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(slot, item);
#endif
    if (FuncDropNoCostManager::getInstance().mutex2) origin(slot, item);
    else FuncDropNoCostManager::getInstance().mutex2 = true;
}

void FuncDropNoCostManager::droppernocostHook(bool bl) {
    if (bl) {
        CoralFansDropperNoCostHook::hook();
        CoralFansDropperNoCostHook2::hook();
        CoralFansDropperNoCostHook3::hook();
        CoralFansDropperNoCostHook4::hook();
    } else {
        CoralFansDropperNoCostHook::unhook();
        CoralFansDropperNoCostHook2::unhook();
        CoralFansDropperNoCostHook3::unhook();
        CoralFansDropperNoCostHook4::unhook();
    }
}
} // namespace coral_fans::functions