#pragma once

#include <functional>
#include <list>

namespace coral_fans::my_schedule {
class MySchedule {
private:
    int now = 0;
    struct ScheduleUnit {
        int                             interval;
        int                             count;
        int                             left_circle_time;
        std::function<bool(int&, int&)> task;

        ScheduleUnit(int delay, int times, int circle_time, std::function<bool(int& interval, int& count)> _task)
        : interval(delay),
          count(times),
          left_circle_time(circle_time),
          task(_task) {}
    };
    std::list<ScheduleUnit> scheduleList[128];

    MySchedule() {}

public:
    static MySchedule& getSchedule() {
        static MySchedule instance;
        return instance;
    }

    void update() {
        now++;
        now &= 0x7f;
        
        // 处理当前槽位中所有 left_circle_time == 0 的任务
        auto it = scheduleList[now].begin();
        while (it != scheduleList[now].end()) {
            if (it->left_circle_time == 0) {
                // 执行任务
                bool shouldContinue = it->task(it->interval, it->count);
                
                if (shouldContinue) {
                    // 重新计算槽位
                    int slot = (it->interval + now) & 0x7f;
                    int circle_time = it->interval >> 7;
                    
                    // 更新任务的 circle_time
                    it->left_circle_time = circle_time;
                    
                    if (slot != now) {
                        // 移动到新的槽位
                        scheduleList[slot].splice(scheduleList[slot].end(), scheduleList[now], it);
                        // it 现在指向新槽位，但我们需要继续处理原槽位
                        it = scheduleList[now].begin();
                        continue;
                    }
                    // 如果槽位相同，保留在当前槽位，继续处理下一个
                    ++it;
                } else {
                    // 删除任务
                    it = scheduleList[now].erase(it);
                }
            } else {
                ++it;
            }
        }
        
        // 减少当前槽位所有任务的 left_circle_time
        for (auto& unit : scheduleList[now]) {
            unit.left_circle_time -= 1;
        }
    }

    void add(std::function<bool(int&, int&)> task, int delay = 1, int times = 0) {
        int slot = (delay + now) & 0x7f;
        int circle_time = delay >> 7;
        scheduleList[slot].emplace_back(delay, times, circle_time, task);
    }

    MySchedule(const MySchedule&) = delete;
    MySchedule& operator=(const MySchedule&) = delete;
};
} // namespace coral_fans::my_schedule
