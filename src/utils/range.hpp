// -*- C++ -*-
// misc.hpp --
//

#pragma once

#include "exceptions.hpp"

#include <limits>
#include <type_traits>


namespace lucid
{
    template <typename T>
    constexpr auto
    range(const T& lower, const T& upper)
    {
		static_assert(std::is_arithmetic_v<T>, "T is not arithmetic!");

        if(lower >= upper)
            throw incorrect_range<T>{lower, upper};

        return [=](const T& val) constexpr
               { return val >= lower && val <= upper; };
    }
}
