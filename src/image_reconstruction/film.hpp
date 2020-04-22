// -*- C++ -*-
// film.hpp
//

#pragma once

#include <base/types.hpp>

namespace lucid
{
constexpr real
ratio(const Vec2u& res) noexcept
{
    const auto& [w, h] = res;
    return static_cast<real>(w) / static_cast<real>(h);
}

template <typename Image>
struct Film
{
    Image img;
    Vec2  res;
    Vec2  pixel_size;

    Film(const Vec2u& res_u, const real& _ratio) noexcept :
        img(res_u), res(res_u), pixel_size(Vec2(_ratio, 1_r) / res)
    {
    }

    Film(const Vec2u& res_u) noexcept :
        img(res_u), res(res_u), pixel_size(Vec2(ratio(res_u), 1_r) / res)
    {
    }

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
            transform(static_cast<real (*)(real)>(math::floor), (sample_pos + 0.5_r) * res));
    }
};
} // namespace lucid
