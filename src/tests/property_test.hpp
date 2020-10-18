// -*- C++ -*-
// property_test.hpp
//

#pragma once

#include <base/vector.hpp>

#include <fmt/ranges.h>

#include <string_view>
#include <utility>

template <typename Testing, typename Property, typename Generator>
bool
test_property(const size_t     n,
              const double     threshold,
              std::string_view property_name,
              Generator&&      generator,
              Testing&&        testing,
              Property&&       property)
{
    size_t sum = 0;
    for(size_t i = 0; i < n; ++i)
    {
        auto&& feed    = generator();
        using FeedType = std::decay_t<decltype(feed)>;
        const bool result =
            property(testing(std::forward<FeedType>(feed)), std::forward<FeedType>(feed));
        sum += static_cast<size_t>(result);
    }

    const auto error = static_cast<double>(sum) / static_cast<double>(n);

    const auto                  ret     = error > threshold;
    static const constexpr char RED[]   = "\033[31m";
    static const constexpr char GREEN[] = "\033[32m";
    static const constexpr char RESET[] = "\033[0m";
    if(ret)
        fmt::print("{}FAIL{} {:.3f}% {}\n", RED, RESET, error * 100, property_name);
    else
        fmt::print("{}OK{} {:.3f}% {}\n", GREEN, RESET, error * 100, property_name);

    return ret > 0;
}

template <size_t I, typename A, typename B>
constexpr bool
not_equal(A&& a, B&& b) noexcept
{
    return std::get<I>(std::forward<A>(a)) != std::get<I>(std::forward<B>(b));
}

template <typename A, typename B, size_t... I>
constexpr bool
assert_tuples(A&& a, B&& b, std::index_sequence<I...>) noexcept
{ return (false || ... || not_equal<I>(std::forward<A>(a), std::forward<B>(b))); }
