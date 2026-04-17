#include "FuncManager.h"
#include "coral_fans/CoralFans.h"


#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include <thread>


#include <string>


namespace coral_fans::functions {
void ContainerOpenManager::sendContentToPlayer(Container* container) {
    using ll::i18n_literals::operator""_tr;
    auto        slots = container->getSlots();
    std::string contentStr;
    int         cnt = 0;
    for (auto item : slots) {
        if (!item->isNull()) {
            ++cnt;
            std::string name = item->getCustomName();
            if (name.empty()) name = item->getName();
            if (name.size() > 50) name = name.substr(0, 50) + "..."; // prevent overflow attack
            contentStr += "§6" + name + "§2(" + std::to_string(item->mCount) + ")§r, ";
        }
    }
    if (cnt == 0) {
        this->player->sendMessage("translate.containerreader.null"_tr());
    } else {
        this->player->sendMessage(
            "translate.containerreader.info"_tr(cnt) + contentStr.substr(0, contentStr.size() - 2)
        );
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerInteractBlockHook,
    HookPriority::Normal,
    GameMode,
    &GameMode::$useItemOn,
    InteractionResult,
    ItemStack&      item,
    BlockPos const& blockPos,
    uchar           face,
    Vec3 const&     clickPos,
    Block const*    block,
    bool            isFirstEvent
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(item, blockPos, face, clickPos, block, isFirstEvent);
#endif
    auto& containerOpenManager  = ContainerOpenManager::getInstance();
    containerOpenManager.player = &this->mPlayer;
    auto res                    = origin(item, blockPos, face, clickPos, block, isFirstEvent);
    containerOpenManager.player = nullptr;
    return res;
}

LL_TYPE_INSTANCE_HOOK(
    CoralFansForceOpenHook,
    ll::memory::HookPriority::Normal,
    ChestBlockActor,
    &ChestBlockActor::$_canOpenThis,
    bool,
    BlockSource& region
) {
#ifdef LL_PLAT_C
    if (std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin(region);
#endif
    auto& containerOpenManager = ContainerOpenManager::getInstance();
    if (containerOpenManager.forceOpen) return true;
    auto res = origin(region);
    if (!res && containerOpenManager.player
        && CoralFans::getInstance().getConfigDb()->get(
               std::format("functions.players.{}.containerreader", containerOpenManager.player->getUuid().asString())
           ) == "true")
        containerOpenManager.sendContentToPlayer(this->getContainer());
    return res;
}

void ContainerOpenManager::hook() {
    auto& configDb = CoralFans::getInstance().getConfigDb();
    if (configDb->get("functions.global.containerreader") == "true") {
        PlayerInteractBlockHook::hook();
        CoralFansForceOpenHook::hook();
    } else if (configDb->get("functions.global.forceopen") == "true") {
        CoralFansForceOpenHook::hook();
        PlayerInteractBlockHook::unhook();
    } else {
        PlayerInteractBlockHook::unhook();
        CoralFansForceOpenHook::unhook();
    }
}
} // namespace coral_fans::functions