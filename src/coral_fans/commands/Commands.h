#pragma once

#include "coral_fans/Config.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/world/actor/player/Player.h"
#include <optional>


namespace coral_fans::commands {
void registerCoralfansCommand();
void registerTickCommand(config::CommandConfigStruct&);
void registerFuncCommand(config::CommandConfigStruct&);
void registerSelfCommand(config::CommandConfigStruct&);
void registerHsaCommand(config::CommandConfigStruct&);
void registerCounterCommand(config::CommandConfigStruct&);
void registerProfCommand(config::CommandConfigStruct&);
void registerSlimeCommand(config::CommandConfigStruct&);
void registerVillageCommand(config::CommandConfigStruct&);
void registerRotateCommand(config::CommandConfigStruct&);
void registerDataCommand(config::CommandConfigStruct&);
void registerCfhudCommand(config::CommandConfigStruct&);
void registerLogCommand(config::CommandConfigStruct&);
void registerCalculateCommand(config::CommandConfigStruct&);
void registerMineruleCommand(config::CommandConfigStruct&);
void registerFreeCameraCommand(config::CommandConfigStruct&);
void registerNoclipCommand(config::CommandConfigStruct&);
void registerLocateCommand(config::CommandConfigStruct& permission);

std::optional<Player*> tryGetPlayer(CommandOrigin const& origin);
} // namespace coral_fans::commands