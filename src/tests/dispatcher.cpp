// -*- C++ -*-
// thread_pool.cpp
#include <utils/dispatcher.hpp>

#include <fmt/format.h>

using namespace lucid;

struct Test
{
    using signature = type_sequence<int, int>;

    constexpr int
    operator()(const int a) const noexcept
    {
        return a * 10;
    }
};

constexpr Test test_func;

int
main()
{
    const int  n = 1000;
    Dispatcher dispatcher(n, std::thread::hardware_concurrency(), test_func);

    int expected = 0;
    int got      = 0;

    for(int i = 0; i < n; ++i)
    {
        if(dispatcher.try_submit(i)) fmt::print("Submitted {}\n", i);
        expected += test_func(i);
    }

    while(const auto i = dispatcher.fetch_result<Test>())
    {
        got += i.value();
        fmt::print("fetched {}\n", i.value());
    }

    const int ret = got != expected;

    if(ret) fmt::print("Expected {}. Got {}\n", expected, got);

    return ret;
}
