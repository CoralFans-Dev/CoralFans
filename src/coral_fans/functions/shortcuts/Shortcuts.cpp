#include "Shortcuts.h"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
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
#include "mc/server/commands/CommandRegistry.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include <memory>

#ifdef LL_PLAT_C
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/service/TargetedBedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "mc/client/game/ClientInstance.h"
#include "mc/client/player/LocalPlayer.h"
#endif


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

void ShortcutsManager::registerShortcutsListener() {
    // useon
    auto& eventListeners = CoralFans::getInstance().getEventListeners();
    eventListeners.emplace(
        ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerInteractBlockEvent>(
            [this](ll::event::player::PlayerInteractBlockEvent& event) {
                if (!::antiShake(event.self(), event.blockPos())) return;
                bool cancel = false;
                for (auto& useon : useons) {
                    if (utils::removeMinecraftPrefix(event.item().getTypeName()) != useon.item) continue;
                    if (!event.block() || utils::removeMinecraftPrefix(event.block()->getTypeName()) != useon.block)
                        continue;
                    auto mc = ll::service::getMinecraft();
                    if (mc) {
                        for (auto& action : useon.actions) {
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
                                std::to_string(event.block()->mBlockType->getVariant(event.block()))
                            );
                            command =
                                ll::string_utils::replaceAll(command, "{blockx}", std::to_string(event.blockPos().x));
                            command =
                                ll::string_utils::replaceAll(command, "{blocky}", std::to_string(event.blockPos().y));
                            command =
                                ll::string_utils::replaceAll(command, "{blockz}", std::to_string(event.blockPos().z));
                            CommandContext context = CommandContext(
                                command,
                                std::make_unique<PlayerCommandOrigin>(
                                    event.self().getLevel(),
                                    event.self().getOrCreateUniqueID()
                                ),
                                static_cast<int>(CurrentCmdVersion::Latest)
                            );
                            [[maybe_unused]] MCRESULT unused = mc->mCommands->executeCommand(context, false);
                        }
                    }
                    cancel |= useon.intercept;
                }
                if (cancel) event.cancel();
            }
        )
    );

    // use
    eventListeners.emplace(ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerUseItemEvent>(
        [this](ll::event::player::PlayerUseItemEvent& event) {
            bool cancel = false;
            for (auto& use : uses) {
                if (utils::removeMinecraftPrefix(event.item().getTypeName()) != use.item) continue;
                auto mc = ll::service::getMinecraft();
                if (mc)
                    for (auto& action : use.actions) {
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
                            std::make_unique<PlayerCommandOrigin>(
                                event.self().getLevel(),
                                event.self().getOrCreateUniqueID()
                            ),
                            static_cast<int>(CurrentCmdVersion::Latest)
                        );
                        mc->mCommands->executeCommand(context, false);
                    }
                cancel |= use.intercept;
            }
            if (cancel) event.cancel();
        }
    ));

    // destroy
    eventListeners.emplace(
        ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
            [this](ll::event::player::PlayerDestroyBlockEvent& event) {
                bool        cancel = false;
                const auto& item   = event.self().getSelectedItem();
                for (auto& destroy : destroys) {
                    if (utils::removeMinecraftPrefix(item.getTypeName()) != destroy.item) continue;
                    auto mc = ll::service::getMinecraft();
                    if (mc)
                        for (auto& action : destroy.actions) {
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
                                std::make_unique<PlayerCommandOrigin>(
                                    event.self().getLevel(),
                                    event.self().getOrCreateUniqueID()
                                ),
                                static_cast<int>(CurrentCmdVersion::Latest)
                            );
                            mc->mCommands->executeCommand(context, false);
                        }
                    cancel |= destroy.intercept;
                }
                if (cancel) event.cancel();
            }
        )
    );

#ifdef LL_PLAT_C
    eventListeners.emplace(ll::event::EventBus::getInstance().emplaceListener<ll::event::KeyInputEvent>(
        [this](ll::event::KeyInputEvent& event) {
            bool cancel         = false;
            auto clientInstance = ll::service::getClientInstance();
            if (!clientInstance) [[unlikely]]
                return;
            if (clientInstance->isShowingMenu()) return;
            auto localPlayer = clientInstance->getLocalPlayer();
            if (!localPlayer) [[unlikely]]
                return;
            for (auto& keyBoard : keyBoards) {
                if (keyBoard.keyCode != event.keyCode() || keyBoard.isDown != event.isDown()) continue;
                ll::thread::ServerThreadExecutor::getDefault().execute([playername = *localPlayer->mName,
                                                                        actions    = keyBoard.actions] {
                    auto level = ll::service::getLevel();
                    if (!level.has_value()) [[unlikely]]
                        return;
                    for (auto& action : actions) {
                        auto pl = ll::service::getLevel()->getPlayer(playername);
                        if (pl) {
                            auto command = ll::string_utils::replaceAll(action, "{selfname}", pl->getRealName());
                            command =
                                ll::string_utils::replaceAll(command, "{selfx}", std::to_string(pl->getPosition().x));
                            command =
                                ll::string_utils::replaceAll(command, "{selfy}", std::to_string(pl->getPosition().y));
                            command =
                                ll::string_utils::replaceAll(command, "{selfz}", std::to_string(pl->getPosition().z));
                            CommandContext context = CommandContext(
                                command,
                                std::make_unique<PlayerCommandOrigin>(level, pl->getOrCreateUniqueID()),
                                static_cast<int>(CurrentCmdVersion::Latest)
                            );
                            auto mc = ll::service::getMinecraft();
                            if (mc) mc->mCommands->executeCommand(context, false);
                        }
                    }
                });
                cancel |= keyBoard.intercept;
            }
            if (cancel) event.cancel();
        }
    ));
#endif
}

void ShortcutsManager::registerShortcutsCommand() {
    using ll::i18n_literals::operator""_tr;
    for (auto& _command : commands) {
        auto& cmd = ll::command::CommandRegistrar::getInstance(false)
                        .getOrCreateCommand(_command.command, _command.description, _command.permission);
        cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
            COMMAND_CHECK_PLAYER
            auto mc = ll::service::getMinecraft();
            if (mc)
                for (auto& action : _command.actions) {
                    auto command = ll::string_utils::replaceAll(action, "{selfname}", player->getRealName());
                    command = ll::string_utils::replaceAll(command, "{selfx}", std::to_string(player->getPosition().x));
                    command = ll::string_utils::replaceAll(command, "{selfy}", std::to_string(player->getPosition().y));
                    command = ll::string_utils::replaceAll(command, "{selfz}", std::to_string(player->getPosition().z));
                    CommandContext context = CommandContext(
                        command,
                        std::make_unique<PlayerCommandOrigin>(player->getLevel(), player->getOrCreateUniqueID()),
                        static_cast<int>(CurrentCmdVersion::Latest)
                    );
                    mc->mCommands->executeCommand(context, false);
                }
        });
    }
}

void ShortcutsManager::loadData() {
    auto const& commandregistry = ll::service::getCommandRegistry();
    auto&       shortcutConfig  = CoralFans::getInstance().getConfig().shortcut;

    // useons
    for (auto& useon : shortcutConfig.useons) {
        if (!useon.enable || useon.item == "") continue;
        for (auto action : useon.actions) {
            if (!commandregistry->findCommand(action)) continue; // 如果action中有一条未注册，则不会加入到shortcuts中
        }
        this->useons.push_back(useon);
    }

    // uses
    for (auto& use : shortcutConfig.uses) {
        if (!use.enable || use.item == "") continue;
        for (auto action : use.actions) {
            if (!commandregistry->findCommand(action)) continue; // 如果action中有一条未注册，则不会加入到shortcuts中
        }
        this->uses.push_back(use);
    }

    // destroys
    for (auto& destroy : shortcutConfig.destroys) {
        if (!destroy.enable || destroy.item == "") continue;
        for (auto action : destroy.actions) {
            if (!commandregistry->findCommand(action)) continue; // 如果action中有一条未注册，则不会加入到shortcuts中
        }
        this->destroys.push_back(destroy);
    }

    // commands
    for (auto& command : shortcutConfig.commands) {
        if (!command.enable || command.command == "") continue;
        for (auto action : command.actions) {
            if (!commandregistry->findCommand(action)) continue; // 如果action中有一条未注册，则不会加入到shortcuts中
        }
        this->commands.push_back(command);
    }

#ifdef LL_PLAT_C
    // key board
    for (auto& command : shortcutConfig.keyBoards) {
        if (!command.enable || command.keyCode == 0) continue;
        for (auto action : command.actions) {
            if (!commandregistry->findCommand(action)) continue; // 如果action中有一条未注册，则不会加入到shortcuts中
        }
        this->keyBoards.push_back(command);
    }
#endif
}

void ShortcutsManager::clear() {
    this->useons.clear();
    this->uses.clear();
    this->destroys.clear();
    this->commands.clear();
#ifdef LL_PLAT_C
    this->keyBoards.clear();
#endif
}
} // namespace coral_fans::functions