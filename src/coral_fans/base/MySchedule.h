#pragma once

#include <functional>

namespace coral_fans::my_schedule {
class MySchedule {
private:
    int now = 0;
    struct SchduleUnit {
        int                             interval;
        int                             count;
        int                             left_circle_time;
        SchduleUnit*                    next = nullptr;
        std::function<bool(int&, int&)> task;

        SchduleUnit(int delay, int times, int circle_time, std::function<bool(int& interval, int& count)> _task)
        : interval(delay),
          count(times),
          left_circle_time(circle_time),
          task(_task) {}
        void insert_after(SchduleUnit* unit) {
            unit->next = this->next;
            this->next = unit;
        }
    };
    SchduleUnit* schduleList[128];

    MySchedule() { std::memset(schduleList, 0, sizeof(schduleList)); }

public:
    [[nodiscard]] static MySchedule& getSchedule() {
        static MySchedule instance;
        return instance;
    }

    void update() {
        now++;
        now &= 0x7f;
        while (schduleList[now] && !schduleList[now]->left_circle_time) {
            SchduleUnit* unit = schduleList[now];
            if (schduleList[now]->task(schduleList[now]->interval, schduleList[now]->count)) {
                schduleList[now] = unit->next;
                int slot         = (unit->interval + now) & 0x7f;
                int circle_time  = unit->interval >> 7;
                if (!schduleList[slot]) {
                    schduleList[slot] = unit;
                    return;
                }
                SchduleUnit* tem = schduleList[slot];
                while (tem->next && tem->next->left_circle_time <= circle_time) tem = tem->next;
                tem->insert_after(unit);
            } else {
                schduleList[now] = unit->next;
                delete unit;
            }
        }
        SchduleUnit* tem = schduleList[now];
        while (tem) {
            tem->left_circle_time -= 1;
            tem                    = tem->next;
        }
    }

    void add(std::function<bool(int&, int&)> task, int delay = 1, int times = 0) {
        int slot        = (delay + now) & 0x7f;
        int circle_time = delay >> 7;
        if (!schduleList[slot]) {
            schduleList[slot] = new SchduleUnit(delay, times, circle_time, task);
            return;
        }
        SchduleUnit* tem = schduleList[slot];
        while (tem->next && tem->next->left_circle_time <= circle_time) tem = tem->next;
        tem->insert_after(new SchduleUnit(delay, times, circle_time, task));
    }

    void clear() {
        for (int i = 0; i < 128; ++i) {
            SchduleUnit* current = schduleList[i];
            while (current) {
                schduleList[i] = current->next;
                delete current;
                current = schduleList[i];
            }
        }
    }

    MySchedule(const MySchedule&)            = delete;
    MySchedule& operator=(const MySchedule&) = delete;
    ~MySchedule() { clear(); };
};
} // namespace coral_fans::my_schedule