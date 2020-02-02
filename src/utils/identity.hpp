// -*- C++ -*-
// identity.hpp
//

#pragma once

#include <utility>

namespace lucid
{
    struct identity
    {
        template <typename T>
        constexpr T&&
        operator()(T&& t) const noexcept
        { return std::forward<T>(t); }
    };
}
