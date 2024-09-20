#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
namespace coral_fans::commands {

void registerSpCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& spCommand = ll::command::CommandRegistrar::getInstance()
                          .getOrCreateCommand("sp", "commands.sp.description"_tr(), permission);
}

} // namespace coral_fans::commands