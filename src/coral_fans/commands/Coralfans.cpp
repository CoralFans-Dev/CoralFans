#include "coral_fans/base/Mod.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"

#include <string>

namespace coral_fans::commands {

void registerCoralfansCommand() {
    // reg cmd
    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand("coralfans", "CoralFans Mod", CommandPermissionLevel::Any);

    // version
    command.overload().text("version").execute([](CommandOrigin const&, CommandOutput& output) {
        output.success(coral_fans::mod().VERSION);
#ifdef COMMITID
        output.success("Commit ID: {}", COMMITID);
#endif
    });
}

} // namespace coral_fans::commands