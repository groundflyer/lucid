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

template <typename T, size_t N>
std::ostream&
operator<<(std::ostream& os, const std::array<T, N>& rhs) noexcept
{
    os << '[';
    for (size_t i = 0; i < N - 1; ++i)
        os << rhs[i] << ", ";
    os << rhs[N-1] << ']';
    return os;
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

template<typename Tuple, size_t... I>
std::ostream&
print_tuple(std::ostream& os, const Tuple& t, std::index_sequence<I...>)
{
    os << '(';
    (..., (os << (I == 0? "" : ", ") << std::get<I>(t)));
    os << ')';
    return os;
}

template<typename... Ts>
std::ostream&
operator<<(std::ostream& os, const std::tuple<Ts...>& t)
{
    return print_tuple(os, t, std::make_index_sequence<sizeof...(Ts)>());
}

template <typename T1, typename T2>
std::ostream&
operator<<(std::ostream& os, std::pair<T1, T2> arg)
{
    os << '(' << arg.first << ", " << arg.second << ')';
    return os;
}

void init_log()
{
    auto log_ok = spdlog::stdout_color_st("ok");
    auto log_fail = spdlog::stderr_color_st("fail");
    auto log_debug = spdlog::stdout_logger_st("debug");
    log_ok->set_pattern("%v: %^OK%$");
    log_fail->set_pattern("%v: %^FAIL%$");
    log_debug->set_pattern("%v");
}

template <typename Property, typename Assertion, typename Generator>
auto test_property(std::string_view property_name,
                   Generator&& generator,
                   Property&& property,
                   Assertion&& assertion,
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
            log_debug->debug("Test '{}' failed with values:\n"
                            "{}\n{}", property_name, feed, prop);
        }

        ret += result;
    }

    if(ret)
        log_fail->error("{}", property_name);
    else
        log_ok->info("{}", property_name);

    return ret > 0;
}
