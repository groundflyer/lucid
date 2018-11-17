// -*- C++ -*-
// property_test.hpp
//

#pragma once

#include <base/vector.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/fmt/ostr.h>

#include <tuple>
#include <utility>
#include <iostream>
#include <string_view>

template <typename T, typename ... Args>
auto construct(std::tuple<Args...> args)
{
    return apply([](auto&&... args){ return T(args...); }, args);
}

template <typename T, size_t N,
          template <typename, size_t> typename Container>
std::ostream&
operator<<(std::ostream& os, const yapt::Vector<T, N, Container>& rhs) noexcept
{
    os << '[';
    for (size_t i = 0; i < N - 1; ++i)
        os << rhs[i] << ", ";
    os << rhs[N-1] << ']';

    return os;
}

template <typename ... Args>
std::ostream&
operator<<(std::ostream& os, std::tuple<Args...> args)
{
    std::apply([&os](auto&&... vals){ ((os << ' ' << vals), ...); }, args);
    return os;
}

template <typename T1, typename T2>
std::ostream&
operator<<(std::ostream& os, std::pair<T1, T2> arg)
{
    os << arg.first << ' ' << arg.second;
    return os;
}

template <typename Property, typename Assertion, typename Generator>
auto test_property(std::string_view property_name,
                   Property&& property,
                   Assertion&& assertion,
                   Generator&& generator,
                   size_t n = 10000) noexcept
{
    auto log_ok = spdlog::get("ok");
    auto log_fail = spdlog::get("fail");
    auto log_debug = spdlog::get("debug");

    size_t ret = 0;
    for(size_t i = 0; i < n; ++i)
    {
        decltype(auto) feed = generator();
        decltype(auto) prop = property(feed);
        decltype(auto) result = assertion(prop, feed);
        if(result)
        {
            log_debug->debug("Test '{}'\n"
                             "failed with feed: {}\n"
                             "and property value: {}",
                             property_name, feed, prop);
        }

        ret += result;
    }

    if(ret)
        log_fail->error("{}", property_name);
    else
        log_ok->info("{}", property_name);

    return ret > 0;
}
