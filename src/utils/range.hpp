// -*- C++ -*-
// misc.hpp --
//

#pragma once

#include <limits>
#include <type_traits>

namespace lucid
{
template <typename T>
constexpr auto
range(T lower, T upper) noexcept
{
    static_assert(std::is_arithmetic_v<T>, "T is not arithmetic!");

    if(lower >= upper) std::swap(lower, upper);

    return [=](const T& val) constexpr { return val >= lower && val < upper; };
}

template <typename T>
constexpr auto
inclusive_range(T lower, T upper) noexcept
{
    static_assert(std::is_arithmetic_v<T>, "T is not arithmetic!");

    if(lower >= upper) std::swap(lower, upper);

    return [=](const T& val) constexpr { return val >= lower && val <= upper; };
}
} // namespace lucid
