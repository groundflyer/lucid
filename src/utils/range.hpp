// -*- C++ -*-
// misc.hpp --
//

#pragma once

#include <algorithm>
#include <limits>
#include <type_traits>

namespace lucid
{
template <typename T>
constexpr auto
range(T lower, T upper) noexcept
{
    static_assert(std::is_arithmetic_v<T>, "T is not arithmetic!");
    std::tie(lower, upper) = std::minmax(lower, upper);
    return [=](const T& val) constexpr { return val >= lower && val <= upper; };
}
} // namespace lucid
