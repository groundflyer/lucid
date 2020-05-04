// -*- C++ -*-
// film.hpp
//

#pragma once

#include <base/rng.hpp>
#include <base/types.hpp>

namespace lucid
{
// sample a random position inside square
template <typename Generator>
Vec2
sample_pixel(Generator& g, const real _pixel_width, const Vec2& pos) noexcept
{
    return (pos - _pixel_width * 0.5_r) + Vec2(rand<real, 2>(g)) * _pixel_width;
}
} // namespace lucid
