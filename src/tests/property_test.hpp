// -*- C++ -*-
// property_test.hpp
//

#pragma once

#include <base/vector.hpp>
#include <base/matrix.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/fmt/ostr.h>

#include <tuple>
#include <sstream>
#include <utility>
#include <iostream>
#include <string_view>

using namespace fmt::literals;

template <typename Construct, typename ... Args>
auto construct(std::tuple<Args...> args)
{ return apply([](auto&&... args){ return Construct(args...); }, args); }

template <typename Construct, typename T, size_t N>
auto construct(const std::array<T, N>& args)
{ return apply([](auto&&... args){ return Construct(args...); }, args); }

namespace yapt
{
    template <typename T, size_t N,
              template <typename, size_t> typename Container>
    std::ostream&
    operator<<(std::ostream& os, const Vector<T, N, Container>& rhs) noexcept
    {
        os << '[';
        for (size_t i = 0; i < N - 1; ++i)
            os << rhs[i] << ", ";
        os << rhs[N - 1] << ']';

        return os;
    }

    template <typename T, size_t R, size_t C,
			  template <typename, size_t> typename Container>
    std::ostream&
    operator<<(std::ostream& os, const Matrix<T, R, C, Container>& rhs) noexcept
    {
        os << '[';
        for (size_t i = 0; i < R - 1; ++i)
            os << rhs[i] << ", ";
        os << rhs[R - 1] << ']';

        return os;
    }
}

namespace std
{
template <typename T, size_t N>
ostream&
operator<<(std::ostream& os, const array<T, N>& rhs) noexcept
{
    os << '[';
    for (size_t i = 0; i < N - 1; ++i)
        os << rhs[i] << ", ";
    os << rhs[N-1] << ']';
    return os;
}

template<typename Tuple, size_t... I>
ostream&
print_tuple(ostream& os, const Tuple& t, index_sequence<I...>)
{
    os << '(';
    (..., (os << (I == 0? "" : ", ") << get<I>(t)));
    os << ')';
    return os;
}

template<typename... Ts>
ostream&
operator<<(ostream& os, const tuple<Ts...>& t)
{
    return print_tuple(os, t, make_index_sequence<sizeof...(Ts)>());
}

template <typename T1, typename T2>
ostream&
operator<<(std::ostream& os, pair<T1, T2> arg)
{
    os << '(' << arg.first << ", " << arg.second << ')';
    return os;
}
}

void init_log(const bool debug = false)
{
    auto log_ok = spdlog::stdout_color_st("ok");
    auto log_fail = spdlog::stderr_color_st("fail");
    auto log_debug = spdlog::stdout_logger_st("debug");
    log_ok->set_pattern("%v: %^OK%$");
    log_fail->set_pattern("%v: %^FAIL%$");
    log_debug->set_pattern("%v");
    if(debug)
        log_debug->set_level(spdlog::level::debug);
}

template <typename Property, typename Assertion, typename Generator>
auto
test_property(const size_t n,
              std::string_view property_name,
              Generator&& generator,
              Property&& property,
              Assertion&& assertion)
{
    auto log_ok = spdlog::get("ok");
    auto log_fail = spdlog::get("fail");
    auto log_debug = spdlog::get("debug");

    size_t ret = 0;
    for(size_t i = 0; i < n; ++i)
    {
        decltype(auto) feed = generator();
        decltype(auto) prop = property(feed);
        const bool result = assertion(prop, feed);
        if(result)
        {
            log_debug->debug("Test '{}' failed with values:\n"
                            "F: {}\nP: {}", property_name, feed, prop);
        }

        ret += static_cast<size_t>(result);
    }

    if(ret)
        log_fail->error("{}", property_name);
    else
        log_ok->info("{}", property_name);

    return ret > 0;
}

#define MAKE_TYPEINFO_STRING(TYPE)                    \
    constexpr auto get_typeinfo_string(TYPE) noexcept \
    { return #TYPE; }

MAKE_TYPEINFO_STRING(int)
MAKE_TYPEINFO_STRING(bool)
MAKE_TYPEINFO_STRING(char)
MAKE_TYPEINFO_STRING(long)
MAKE_TYPEINFO_STRING(float)
MAKE_TYPEINFO_STRING(double)
MAKE_TYPEINFO_STRING(size_t)
MAKE_TYPEINFO_STRING(long double)
MAKE_TYPEINFO_STRING(unsigned int)
MAKE_TYPEINFO_STRING(unsigned char)
MAKE_TYPEINFO_STRING(unsigned short)

template <typename T, size_t N>
constexpr auto
get_typeinfo_string(yapt::Vector<T, N, std::array>)
{ return "Vector<{}, {}>"_format(get_typeinfo_string(T{}), N); }

template <typename... Ts>
constexpr auto get_typeinfo_string(std::tuple<Ts...>) noexcept
{ return "{}"_format(tuple(get_typeinfo_string(Ts{})...)); }

template <typename T, size_t N>
auto get_typeinfo_string(std::array<T, N>) noexcept
{
    std::stringstream ss;
    auto typestring = get_typeinfo_string(T{});
    for(size_t i = 0; i < N - 1; ++i)
        ss << typestring << ", ";
    ss << typestring;
    return ss.str();
}

template <size_t I, typename A, typename B>
constexpr bool
not_equal(A&& a, B&& b) noexcept
{ return std::get<I>(std::forward<A>(a)) != std::get<I>(std::forward<B>(b)); }

template <typename A, typename B, size_t... I>
constexpr bool
assert_tuples(A&& a, B&& b, std::index_sequence<I...>) noexcept
{ return (false || ... || not_equal<I>(std::forward<A>(a), std::forward<B>(b))); }
