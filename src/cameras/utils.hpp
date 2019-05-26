// -*- C++ -*-
// utils.hpp
//

#pragma once

#include <base/types.hpp>


namespace lucid
{
    // convert image coordinates to device coordinates
    constexpr auto
    to_device_coords(const Vec2& pos, const Vec2& res) noexcept
    { return (pos - res * 0.5_r) / res[0]; }
}
