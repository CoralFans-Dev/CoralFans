#pragma once

#include <functional>
#include <list>
#include <vector>

namespace coral_fans::timewheel {

class TimeWheel {
private:
    using TaskFn = std::function<bool(void)>;
    struct Task {
        unsigned long long mRotation;
        unsigned long long mDuration;
        TaskFn             mFn;
    };
    std::vector<std::list<Task>> mWheel;
    unsigned long long           mSize;
    unsigned long long           mCurrentSlot;

public:
    TimeWheel(unsigned long long n) : mSize(n), mCurrentSlot(0) { this->mWheel.resize(n); }
    ~TimeWheel() { this->clear(); }
    TimeWheel(const TimeWheel&)            = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;

public:
    inline void clear() { this->mWheel.clear(); }
    inline void add(unsigned long long ticks, TaskFn fn) {
        this->mWheel[(this->mCurrentSlot + ticks) % this->mSize].emplace_back(ticks / this->mSize, ticks, fn);
    }

public:
    void tick() {
        std::list<Task> newTasks;
        for (auto& task : this->mWheel[this->mCurrentSlot])
            if (task.mRotation == 0) {
                if (task.mFn()) this->add(task.mDuration, task.mFn);
            } else {
                --task.mRotation;
                newTasks.emplace_back(task);
            }
        this->mWheel[this->mCurrentSlot] = newTasks;
        this->mCurrentSlot               = (this->mCurrentSlot + 1) % this->mSize;
    }
};

} // namespace coral_fans::timewheel