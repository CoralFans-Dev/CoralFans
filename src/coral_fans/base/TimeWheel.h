#pragma once

#include <functional>
#include <list>
#include <unordered_set>
#include <vector>

namespace coral_fans::timewheel {

class TimeWheel {
private:
    using TaskFn = std::function<bool(unsigned long long)>;
    struct Task {
        unsigned long long mTaskId;
        unsigned long long mRotation;
        unsigned long long mDuration;
        unsigned long long mCallTimes;
        TaskFn             mFn;
    };
    std::vector<std::list<Task>>           mWheel;
    std::unordered_set<unsigned long long> mCancelSet;
    std::unordered_set<unsigned long long> mRunningTasks;
    unsigned long long                     mCurrentId;
    unsigned long long                     mSize;
    unsigned long long                     mCurrentSlot;

public:
    TimeWheel(unsigned long long n) : mCurrentId(1), mSize(n), mCurrentSlot(0) { this->mWheel.resize(n); }
    ~TimeWheel() { this->clear(); }
    TimeWheel(const TimeWheel&)            = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;
    TimeWheel(const TimeWheel&& right) {
        this->mWheel        = std::move(right.mWheel);
        this->mCancelSet    = std::move(right.mCancelSet);
        this->mRunningTasks = std::move(right.mRunningTasks);
        this->mCurrentId    = right.mCurrentId;
        this->mSize         = right.mSize;
        this->mCurrentSlot  = right.mCurrentSlot;
    }

public:
    inline void               clear() { this->mWheel.clear(); }
    inline unsigned long long add(unsigned long long ticks, TaskFn fn) {
        unsigned long long tid = this->mCurrentId++;
        this->mWheel[(this->mCurrentSlot + ticks) % this->mSize].emplace_back(tid, ticks / this->mSize, ticks, 1, fn);
        this->mRunningTasks.insert(tid);
        return tid;
    }
    inline void cancel(unsigned long long tid) { this->mCancelSet.insert(tid); }
    inline void cancelAll() {
        for (auto& i : this->mWheel) i.clear();
    }
    inline bool isRunning(unsigned long long tid) { return this->mRunningTasks.find(tid) != this->mRunningTasks.end(); }
    inline bool isFree() { return this->mRunningTasks.size() == 0; }

private:
    inline void add(Task task) {
        this->mWheel[(this->mCurrentSlot + task.mDuration) % this->mSize]
            .emplace_back(task.mTaskId, task.mDuration / this->mSize, task.mDuration, task.mCallTimes + 1, task.mFn);
    }

public:
    void tick() {
        std::list<Task> newTasks;
        for (auto& task : this->mWheel[this->mCurrentSlot]) {
            auto it = this->mCancelSet.find(task.mTaskId);
            if (it == this->mCancelSet.end()) [[likely]] {
                if (task.mRotation == 0) [[likely]] { // execute
                    if (task.mFn(task.mCallTimes)) this->add(task);
                    else this->mRunningTasks.erase(task.mTaskId);
                } else [[unlikely]] { // wait for next rotation
                    --task.mRotation;
                    newTasks.emplace_back(task);
                }
            } else [[unlikely]] {
                this->mCancelSet.erase(it);
                this->mRunningTasks.erase(task.mTaskId);
            }
        }
        this->mWheel[this->mCurrentSlot] = newTasks;
        this->mCurrentSlot               = (this->mCurrentSlot + 1) % this->mSize;
    }
};

} // namespace coral_fans::timewheel