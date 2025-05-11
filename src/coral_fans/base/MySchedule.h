#include "coral_fans/base/Mod.h"
#include <functional>

namespace coral_fans::my_schedule {
class MySchedule {
private:
    int now = 0;
    struct SchduleUnit {
        int                   left_circle_time;
        SchduleUnit*          next = nullptr;
        std::function<void()> task;

        SchduleUnit(int circle_time, std::function<void()> _task) : left_circle_time(circle_time), task(_task) {}
        void insert_after(SchduleUnit* unit) {
            unit->next = this->next;
            this->next = unit;
        }
    };
    SchduleUnit* schduleList[128];

    MySchedule() { std::memset(schduleList, 0, sizeof(schduleList)); }

public:
    static MySchedule& getSchedule() {
        static MySchedule instance;
        return instance;
    }

    void update() {
        now++;
        now &= 0x7f;
        while (schduleList[now] && !schduleList[now]->left_circle_time) {
            // schduleList[now]->task();
            try {
                schduleList[now]->task(); // 执行任务
            } catch (const std::exception& e) {
                mod().getLogger().error("Exception occurred while executing task: {}", e.what());
            }
            SchduleUnit* tem = schduleList[now];
            schduleList[now] = tem->next;
            delete tem;
        }
        SchduleUnit* tem = schduleList[now];
        while (tem) {
            tem->left_circle_time -= 1;
            tem                    = tem->next;
        }
    }

    void add(int t, std::function<void()> task) {
        int slot        = (t + now) & 0x7f;
        int circle_time = t >> 7;
        if (!schduleList[slot]) {
            schduleList[slot] = new SchduleUnit(circle_time, task);
            return;
        }
        SchduleUnit* tem = schduleList[slot];
        while (tem->next && tem->next->left_circle_time <= circle_time) tem = tem->next;
        tem->insert_after(new SchduleUnit(circle_time, task));
    }

    MySchedule(const MySchedule&)            = delete;
    MySchedule& operator=(const MySchedule&) = delete;
    ~MySchedule() {
        for (int i = 0; i < 128; ++i) {
            SchduleUnit* current = schduleList[i];
            while (current) {
                schduleList[i] = current->next;
                delete current;
                current = schduleList[i];
            }
        }
    };
};
} // namespace coral_fans::my_schedule