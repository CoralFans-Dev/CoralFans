#pragma once

#include <string>

namespace coral_fans::commands {
void registerTickCommand(std::string);
void registerFuncCommand(std::string);
void registerSelfCommand(std::string);
void registerHsaCommand(std::string);
void registerCounterCommand(std::string);
void registerProfCommand(std::string);
void registerSlimeCommand(std::string);
void registerVillageCommand(std::string);
void registerRotateCommand(std::string);
} // namespace coral_fans::commands