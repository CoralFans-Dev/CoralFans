#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/StringUtils.h"
#include "mc/deps/core/utility/MCRESULT.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include <memory>


namespace {

// anti shake from trapdoor-ll
struct UseOnAction {
    uint64_t gameTick = 0;
    BlockPos pos;

    bool operator==(const UseOnAction& rhs) const {
        if (pos != rhs.pos) return false;
        return (gameTick - rhs.gameTick) <= 3;
    }

    bool operator!=(const UseOnAction& rhs) const { return !(rhs == *this); }
};

std::unordered_map<std::string, UseOnAction>& getUseOnCache() {
    static std::unordered_map<std::string, UseOnAction> cache;
    return cache;
}

bool antiShake(const Player& player, const BlockPos& pos) {
    const auto& playerUuid      = player.getUuid().asString();
    uint64_t    gt              = player.getLevel().getCurrentServerTick().tickID;
    auto        useOnAction     = UseOnAction{gt, pos};
    auto        lastUseOnAction = getUseOnCache()[playerUuid];
    if (useOnAction == lastUseOnAction) return false;
    getUseOnCache()[playerUuid] = useOnAction;
    return true;
}

} // namespace

namespace coral_fans::functions {

void registerShortcutsListener() {
    // useon
    ll::event::ListenerPtr playerInteractBlockEventListener;
    playerInteractBlockEventListener =
        coral_fans::mod().getEventBus().emplaceListener<ll::event::player::PlayerInteractBlockEvent>(
            [](ll::event::player::PlayerInteractBlockEvent& event) {
                if (!::antiShake(event.self(), event.blockPos())) return;
                bool cancel = false;
                for (auto& shortcut : coral_fans::mod().getConfig().shortcuts) {
                    if (!shortcut.enable || shortcut.type != "useon" || shortcut.item == "") continue;
                    if (utils::removeMinecraftPrefix(event.item().getTypeName()) != shortcut.item) continue;
                    if (!event.block() || utils::removeMinecraftPrefix(event.block()->getTypeName()) != shortcut.block)
                        continue;
                    auto mc = ll::service::getMinecraft();
                    if (mc)
                        for (auto& action : shortcut.actions) {
                            auto command =
                                ll::string_utils::replaceAll(action, "{selfname}", event.self().getRealName());
                            command = ll::string_utils::replaceAll(
                                command,
                                "{selfx}",
                                std::to_string(event.self().getPosition().x)
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{selfy}",
                                std::to_string(event.self().getPosition().y)
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{selfz}",
                                std::to_string(event.self().getPosition().z)
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{itemname}",
                                event.item().getCustomName().empty() ? event.item().getName()
                                                                     : event.item().getCustomName()
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{itemaux}",
                                std::to_string(event.item().getAuxValue())
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{blockname}",
                                event.block()->buildDescriptionName()
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{blockvariant}",
                                std::to_string(event.block()->getVariant())
                            );
                            command =
                                ll::string_utils::replaceAll(command, "{blockx}", std::to_string(event.blockPos().x));
                            command =
                                ll::string_utils::replaceAll(command, "{blocky}", std::to_string(event.blockPos().y));
                            command =
                                ll::string_utils::replaceAll(command, "{blockz}", std::to_string(event.blockPos().z));
                            CommandContext context = CommandContext(
                                command,
                                std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(event.self())),
                                CommandVersion::CurrentVersion()
                            );
                            mc->getCommands().executeCommand(context, false);
                        }
                    cancel |= shortcut.prevent;
                }
                if (cancel) event.cancel();
            }
        );
    coral_fans::mod().getEventListeners().emplace(playerInteractBlockEventListener);
    // use
    ll::event::ListenerPtr playerUseItemEventListener;
    playerUseItemEventListener = coral_fans::mod().getEventBus().emplaceListener<ll::event::player::PlayerUseItemEvent>(
        [](ll::event::player::PlayerUseItemEvent& event) {
            bool cancel = false;
            for (auto& shortcut : coral_fans::mod().getConfig().shortcuts) {
                if (!shortcut.enable || shortcut.type != "use" || shortcut.item == "") continue;
                if (utils::removeMinecraftPrefix(event.item().getTypeName()) != shortcut.item) continue;
                auto mc = ll::service::getMinecraft();
                if (mc)
                    for (auto& action : shortcut.actions) {
                        auto command = ll::string_utils::replaceAll(action, "{selfname}", event.self().getRealName());
                        command      = ll::string_utils::replaceAll(
                            command,
                            "{selfx}",
                            std::to_string(event.self().getPosition().x)
                        );
                        command = ll::string_utils::replaceAll(
                            command,
                            "{selfy}",
                            std::to_string(event.self().getPosition().y)
                        );
                        command = ll::string_utils::replaceAll(
                            command,
                            "{selfz}",
                            std::to_string(event.self().getPosition().z)
                        );
                        command = ll::string_utils::replaceAll(
                            command,
                            "{itemname}",
                            event.item().getCustomName().empty() ? event.item().getName() : event.item().getCustomName()
                        );
                        command = ll::string_utils::replaceAll(
                            command,
                            "{itemaux}",
                            std::to_string(event.item().getAuxValue())
                        );
                        CommandContext context = CommandContext(
                            command,
                            std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(event.self())),
                            CommandVersion::CurrentVersion()
                        );
                        mc->getCommands().executeCommand(context, false);
                    }
                cancel |= shortcut.prevent;
            }
            if (cancel) event.cancel();
        }
    );
    coral_fans::mod().getEventListeners().emplace(playerUseItemEventListener);
    // destroy
    ll::event::ListenerPtr playerDestroyBlockEventListener;
    playerInteractBlockEventListener =
        coral_fans::mod().getEventBus().emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
            [](ll::event::player::PlayerDestroyBlockEvent& event) {
                bool        cancel = false;
                const auto& item   = event.self().getSelectedItem();
                for (auto& shortcut : coral_fans::mod().getConfig().shortcuts) {
                    if (!shortcut.enable || shortcut.type != "destroy" || shortcut.item == "") continue;
                    if (utils::removeMinecraftPrefix(item.getTypeName()) != shortcut.item) continue;
                    auto mc = ll::service::getMinecraft();
                    if (mc)
                        for (auto& action : shortcut.actions) {
                            auto command =
                                ll::string_utils::replaceAll(action, "{selfname}", event.self().getRealName());
                            command = ll::string_utils::replaceAll(
                                command,
                                "{selfx}",
                                std::to_string(event.self().getPosition().x)
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{selfy}",
                                std::to_string(event.self().getPosition().y)
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{selfz}",
                                std::to_string(event.self().getPosition().z)
                            );
                            command = ll::string_utils::replaceAll(
                                command,
                                "{itemname}",
                                item.getCustomName().empty() ? item.getName() : item.getCustomName()
                            );
                            command =
                                ll::string_utils::replaceAll(command, "{itemaux}", std::to_string(item.getAuxValue()));
                            CommandContext context = CommandContext(
                                command,
                                std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(event.self())),
                                CommandVersion::CurrentVersion()
                            );
                            mc->getCommands().executeCommand(context, false);
                        }
                    cancel |= shortcut.prevent;
                }
                if (cancel) event.cancel();
            }
        );
    coral_fans::mod().getEventListeners().emplace(playerDestroyBlockEventListener);
}

void registerShortcutsCommand() {
    using ll::i18n_literals::operator""_tr;
    for (auto& shortcut : coral_fans::mod().getConfig().shortcuts) {
        if (!shortcut.enable || shortcut.type != "command" || shortcut.command == "") continue;
        auto& cmd = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand(shortcut.command, shortcut.description, shortcut.permission);
        cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
            COMMAND_CHECK_PLAYER
            auto mc = ll::service::getMinecraft();
            if (mc)
                for (auto& action : shortcut.actions) {
                    auto command = ll::string_utils::replaceAll(action, "{selfname}", player->getRealName());
                    command = ll::string_utils::replaceAll(command, "{selfx}", std::to_string(player->getPosition().x));
                    command = ll::string_utils::replaceAll(command, "{selfy}", std::to_string(player->getPosition().y));
                    command = ll::string_utils::replaceAll(command, "{selfz}", std::to_string(player->getPosition().z));
                    CommandContext context = CommandContext(
                        command,
                        std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(*player)),
                        CommandVersion::CurrentVersion()
                    );
                    mc->getCommands().executeCommand(context, false);
                }
        });
    }
}

} // namespace coral_fans::functions