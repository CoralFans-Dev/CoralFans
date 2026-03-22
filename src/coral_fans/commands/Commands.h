#pragma once

#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"
#include <optional>


namespace coral_fans::commands {
void                   registerCoralfansCommand();
void                   registerTickCommand(CommandPermissionLevel);
void                   registerFuncCommand(CommandPermissionLevel);
void                   registerSelfCommand(CommandPermissionLevel);
void                   registerHsaCommand(CommandPermissionLevel);
void                   registerCounterCommand(CommandPermissionLevel);
void                   registerProfCommand(CommandPermissionLevel);
void                   registerSlimeCommand(CommandPermissionLevel);
void                   registerVillageCommand(CommandPermissionLevel);
void                   registerRotateCommand(CommandPermissionLevel);
void                   registerDataCommand(CommandPermissionLevel);
void                   registerCfhudCommand(CommandPermissionLevel);
void                   registerLogCommand(CommandPermissionLevel);
void                   registerCalculateCommand(CommandPermissionLevel);
void                   registerMineruleCommand(CommandPermissionLevel);
void                   registerFreeCameraCommand(CommandPermissionLevel);
void                   registerNoclipCommand(CommandPermissionLevel);
void                   registerLocateCommand(CommandPermissionLevel permission);
std::optional<Player*> tryGetPlayer(CommandOrigin const& origin);
} // namespace coral_fans::commands