// -*- C++ -*-
// film.hpp
//

#pragma once

#include <base/types.hpp>

namespace lucid
{
template <typename Image>
struct Film
{
    Image img;
    Vec2  res;
    Vec2  pixel_size;

    Film(const Vec2u res_u) : img(res_u), res(res_u), pixel_size(Vec2(1_r) / res) {}

    // radius of pixel bounding ellipse
    real
    pixel_radius() const noexcept
    {
        return math::sqrt(sum(pow<2>(pixel_size))) * 0.5_r;
    }

    Vec2
    device_coords(const Vec2u& pixel_pos) const noexcept
    {
        return (Vec2(pixel_pos) - res * 0.5_r) / res + pixel_size * 0.5_r;
    }

    Vec2u
    pixel_coords(const Vec2& sample_pos) const noexcept
    {
        return Vec2u(
            transform(static_cast<real (*)(real)>(math::round), (sample_pos + 0.5_r) * res));
    }
};
} // namespace lucid
