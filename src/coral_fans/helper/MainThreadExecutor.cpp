#include "MainThreadExecutor.h"

#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/base/Containers.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/ErrorUtils.h"
#include "mc/world/Minecraft.h"


#include <functional>
#include <memory>
#include <utility>


namespace coral_fans::helper::thread {
LL_TYPE_INSTANCE_HOOK(MainThreadExecutorHook, ll::memory::HookPriority::Normal, Minecraft, &Minecraft::update, bool) {
    RETURN_IF_NOT_MAIN_THREAD(return origin());
    auto ori = origin();
    if (ori) MainThreadExecutor::getDefault().tick();
    return ori;
}

struct MainThreadExecutor::Impl {
    struct ScheduledWork {
        uint64                                         time;
        std::shared_ptr<ll::data::CancellableCallback> callback;
    };
    struct SwCmp {
        bool operator()(ScheduledWork const& x, ScheduledWork const& y) { return x.time > y.time; }
    };
    // struct Impl {
    ll::ConcurrentQueue<std::function<void()>> works;
    decltype(works)::consumer_token_t          token;

    ll::ConcurrentPriorityQueue<ScheduledWork, SwCmp> scheduledWorks;
    std::atomic_uint64_t                              frame{0};

    Duration maxOnceDuration;
    size_t   checkPack;

    Impl(Duration maxOnceDuration, size_t checkPack)
    : token(works),
      maxOnceDuration(maxOnceDuration),
      checkPack(checkPack) {}
};

MainThreadExecutor::MainThreadExecutor(std::string name, Duration maxOnceDuration, size_t checkPack)
: Executor(std::move(name)),
  impl(std::make_unique<Impl>(maxOnceDuration, checkPack)) {
    MainThreadExecutorHook::hook();
}

MainThreadExecutor::~MainThreadExecutor() { MainThreadExecutorHook::unhook(); }
void MainThreadExecutor::execute(std::function<void()> f) const { impl->works.enqueue(std::move(f)); }

std::shared_ptr<ll::data::CancellableCallback>
MainThreadExecutor::executeAfter(std::function<void()> f, Duration dur) const {
    auto tick = std::chrono::ceil<ll::chrono::ticks>(dur).count();
    if (tick <= 0) {
        execute(std::move(f));
        return nullptr;
    } else {
        auto res = std::make_shared<ll::data::CancellableCallback>(std::move(f));
        impl->scheduledWorks.emplace(impl->frame.load() + tick, res);
        return res;
    }
}

MainThreadExecutor const& MainThreadExecutor::getDefault() {
    static MainThreadExecutor ins("coral_fans_main_thread", std::chrono::milliseconds{30}, 16);
    return ins;
}

void MainThreadExecutor::tick() const {
    auto begin = Clock::now();
    auto now   = impl->frame.load();

    // 处理到期的定时任务
    while (impl->scheduledWorks.try_pop_if([&](Impl::ScheduledWork& w) {
        if (w.time <= now) {
            w.callback->moveTo([&](auto&& fn) {
                impl->works.enqueue(std::move(fn));
                return true;
            });
            return true;
        }
        return false;
    })) {}

    // 处理普通任务，带时间限制
    std::function<void()> f;
    size_t                i = 0;
    while (impl->works.try_dequeue(impl->token, f)) {
        try {
            f();
        } catch (...) {
            CoralFans::getInstance().getSelf().getLogger().error("Error in {}:", getName());
            ll::error_utils::printCurrentException(CoralFans::getInstance().getSelf().getLogger());
        }
        if (++i % impl->checkPack == 0 && Clock::now() - begin > impl->maxOnceDuration) {
            break;
        }
    }
    impl->frame++;
}
} // namespace coral_fans::helper::thread
