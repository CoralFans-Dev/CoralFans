#pragma once

#include "ll/api/coro/Executor.h"

namespace coral_fans::helper::thread {
class MainThreadExecutor final : public ll::coro::Executor {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    MainThreadExecutor(std::string name, Duration maxOnceDuration, size_t checkPack);

    ~MainThreadExecutor() override;

    void execute(std::function<void()>) const override;

    std::shared_ptr<ll::data::CancellableCallback> executeAfter(std::function<void()>, Duration) const override;

    static MainThreadExecutor const& getDefault();

    void tick() const;

    void clear() const;
};
} // namespace coral_fans::helper::thread
