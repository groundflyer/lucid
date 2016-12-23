// -*- C++ -*-
// pi.hpp --
//
// P(n + 1) = P(n) + sin(P(n))

#pragma once

#include <cmath>


namespace yapt
{
    template <typename T>
    const constexpr T PI = static_cast<T>(3.141592653589793);
    // const constexpr T PI = T(std::acos(-1.L));
}
