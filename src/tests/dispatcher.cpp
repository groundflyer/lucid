// -*- C++ -*-
// thread_pool.cpp
#include <utils/dispatcher.hpp>
#include <utils/timer.hpp>

#include <chrono>

#include <fmt/format.h>

using namespace std::chrono_literals;
using namespace lucid;

template <typename Duration, typename Value>
struct TimerTask
{
    Duration pause;
    Value value;

    TimerTask() noexcept {}

    TimerTask(const Duration& p,
         const Value& v) noexcept : pause(p), value(v) {}

    Value
    operator()() const noexcept
    {
        std::this_thread::sleep_for(pause);
        return value;
    }
};

using TestTimerTask = TimerTask<std::chrono::milliseconds, int>;


int main()
{
    Dispatcher<TestTimerTask> dispatcher;

    const int N = 64;

    ElapsedTimer<> timer;

    auto consumer_worker = [&]()
    {
        int sum = 0;
        while (sum < N)
        {
            const auto result = dispatcher.template fetch_result<TestTimerTask>();
            if(result)
            {
                sum += result.value();
                fmt::print("{} ms; Sum: {}\n",
                           std::chrono::duration_cast<std::chrono::milliseconds>(timer.elapsed()).count(),
                           sum);
            }
        }
    };

    std::thread consumer(consumer_worker);

    for (int i = 0; i < N; ++i)
    {
        while (!dispatcher.try_submit(TestTimerTask{1s, 1}))
        {
            fmt::print("Failed to submit task\n");
        }
        fmt::print("Submitted task {}\n", i);
    }

    consumer.join();

    fmt::print("{} results fetched in {} ms\n",
               N,
               std::chrono::duration_cast<std::chrono::milliseconds>(timer.elapsed()).count());


    return 0;
}
