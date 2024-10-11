#pragma once

namespace coral_fans::functions {

void hookTweakersAutoTool(bool);
void hookFunctionsHopperCounter(bool);
void hookTick(bool);
void hookTweakers(bool);
void hookVillage(bool);

inline void hookAll(bool hook) {
    hookTweakersAutoTool(hook);
    hookFunctionsHopperCounter(hook);
    hookTick(hook);
    hookTweakers(hook);
    hookVillage(hook);
}

} // namespace coral_fans::functions