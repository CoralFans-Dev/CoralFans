#include "coral_fans/base/Macros.h"
#include "coral_fans/functions/locate/DuplicatableManager.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerLocateCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& locateCommand = ll::command::CommandRegistrar::getInstance(false)
                              .getOrCreateCommand("cflocate", "command.locate.description"_tr(), permission);

    // locate duplicatable show <bounds|raid|spawn|center|poi|bind> <bool>
    ll::command::CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(
        "duplicatableShowType",
        {
            {"netherite", 0}
    }
    );

    locateCommand.runtimeOverload()
        .text("duplicatable")
        .required("type", ll::command::ParamKind::Enum, "duplicatableShowType")
        .optional("enable", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& duplicatableManager = functions::locate::DuplicatableManager::getInstance();
            auto  showType            = functions::locate::DuplicatableManager::ShowType(0);
            switch (self["type"].get<ll::command::ParamKind::Enum>().index) {
            case 0:
                showType = functions::locate::DuplicatableManager::ShowType::Netherite;
                break;
            }
            if (!static_cast<uint>(showType)) return output.success("command.locate.duplicatable.show.error"_tr());
            if (self["enable"].has_value())
                duplicatableManager.setShowType(showType, self["enable"].get<ll::command::ParamKind::Bool>());
            else duplicatableManager.setShowType(showType, !duplicatableManager.getShowType(showType));
            bool isopen = duplicatableManager.getShowType(showType);
            output.success("command.locate.duplicatable.show.output"_tr(
                self["type"].get<ll::command::ParamKind::Enum>().name,
                isopen ? "true" : "false"
            ));
        });

    locateCommand.runtimeOverload().text("test").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            output.success(functions::locate::DuplicatableManager::getInstance().test(ChunkPos(player->getPosition())));
        }
    );

    functions::locate::DuplicatableManager::hook(true);
}
} // namespace coral_fans::commands