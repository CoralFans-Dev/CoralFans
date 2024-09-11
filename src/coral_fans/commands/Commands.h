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
} // namespace coral_fans::commands