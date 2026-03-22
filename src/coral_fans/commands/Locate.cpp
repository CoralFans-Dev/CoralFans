#include "coral_fans/functions/locate/locateManager.h"

#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerLocateCommand(CommandPermissionLevel permission) { functions::locate::LocateManager::hook(true); }
} // namespace coral_fans::commands