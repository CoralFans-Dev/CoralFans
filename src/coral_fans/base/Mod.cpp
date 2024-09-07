#include "coral_fans/base/Mod.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/schedule/Task.h"
#include <functional>
#include <string_view>

namespace coral_fans {

bool CoralFansMod::addTask(std::string_view name, unsigned tick, std::function<void()> f) {
    if (this->mTaskMap.find(name) == this->mTaskMap.end()) {
        this->mTaskMap[name] = this->mGameTickScheduler.add<ll::schedule::RepeatTask>(ll::chrono::ticks{tick}, f);
        return true;
    } else return false;
}

bool CoralFansMod::removeTask(std::string_view name) {
    if (this->mTaskMap.find(name) != this->mTaskMap.end()) {
        this->mGameTickScheduler.remove(this->mTaskMap[name]);
        this->mTaskMap.erase(name);
        return true;
    } else return false;
}

CoralFansMod& mod() {
    static CoralFansMod coralFansMod;
    return coralFansMod;
}

} // namespace coral_fans