#include "coral_fans/base/Mod.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"

#include "ll/api/i18n/I18n.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/Container.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"
#include "mc/world/level/block/utils/BlockActorType.h"
#include "mc/world/level/dimension/Dimension.h"

#include <string>

namespace {

void SendContentToPlayer(ServerPlayer& pl, Container* container) {
    using ll::i18n_literals::operator""_tr;
    auto        slots = container->getSlots();
    std::string contentStr;
    int         cnt = 0;
    for (auto item : slots) {
        if (!item->isNull()) {
            ++cnt;
            std::string name = item->getName();
            if (name.size() > 50) name = name.substr(0, 50) + "..."; // prevent overflow attack
            contentStr += "§6" + name + "§2(" + std::to_string(item->mCount) + ")§r, ";
        }
    }
    if (cnt == 0) {
        pl.sendMessage("translate.containerreader.null"_tr());
    } else {
        pl.sendMessage("translate.containerreader.info"_tr(cnt) + contentStr.substr(0, contentStr.size() - 2));
    }
}

} // namespace


namespace coral_fans::functions {

void registerContainerReader() {
    ll::event::ListenerPtr evListener;
    auto&                  mod = coral_fans::mod();
    evListener                 = mod.getEventBus().emplaceListener<ll::event::PlayerInteractBlockEvent>(
        [&](ll::event::PlayerInteractBlockEvent& ev) {
            if (mod.getConfigDb()->get(
                    std::format("functions.players.{}.containerreader", ev.self().getUuid().asString())
                )
                != "true")
                return;
            auto&       bs = ev.self().getDimension().getBlockSourceFromMainChunkSource();
            const auto& bl = bs.getBlock(ev.blockPos());
            auto*       ba = bs.getBlockEntity(ev.blockPos());
            if (bl.isContainerBlock() && ba) {
                auto type = ba->getType();
                if (type == BlockActorType::Chest || type == BlockActorType::ShulkerBox) {
                    // chest or shulker_box, check if can be open
                    auto cba = reinterpret_cast<ChestBlockActor*>(ba);
                    if (!cba->canOpen(bs)) {
                        // cannot open, send content to player
                        ::SendContentToPlayer(ev.self(), ba->getContainer());
                    }
                }
            }
        }
    );
    mod.getEventListeners().emplace(evListener);
}

} // namespace coral_fans::functions