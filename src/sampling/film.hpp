// -*- C++ -*-
// film.hpp
//

#pragma once

#include <base/types.hpp>

#include <random>

namespace lucid
{
// sample a random position inside square
template <typename Generator>
Vec2
sample_pixel(Generator& g, const real _pixel_width, const Vec2& pos) noexcept
{
    return (pos - _pixel_width * 0.5_r) +
           Vec2(generate<3>(static_cast<real (*)(Generator&)>(std::generate_canonical<real, 8>),
                            g)) *
               _pixel_width;
}
} // namespace lucid
