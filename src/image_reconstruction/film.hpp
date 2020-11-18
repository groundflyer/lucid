// -*- C++ -*-
// film.hpp
//

#pragma once

#include <base/types.hpp>
#include <utils/range.hpp>

namespace lucid
{
constexpr real
ratio(const Vec2u& res) noexcept
{
    const auto& [w, h] = res;
    return static_cast<real>(w) / static_cast<real>(h);
}

constexpr real
ratio(const Vec2& res) noexcept
{
    const auto& [w, h] = res;
    return w / h;
}

// pixel index to film sample space
constexpr Vec2
sample_space(const Vec2& _res, const real _ratio, const Vec2u& _idx) noexcept
{
    const auto& [c, r] = _idx;
    const auto [w, h]  = _res - 1_r;
    const real sx      = static_cast<real>(c) / w * _ratio - _ratio * 0.5_r;
    const real sy      = static_cast<real>(r) / h - 0.5_r;
    return Vec2{sx, sy};
}

constexpr Vec2
sample_space(const Vec2& _res, const Vec2u& _idx) noexcept
{
    return sample_space(_res, ratio(_res), _idx);
}

Vec2u
pixel_index(const Vec2& _res, const real _ratio, const Vec2& sample_pos) noexcept
{
    const auto& [sx, sy] = sample_pos;
    const auto [w, h]    = _res - 1_r;
    // normalize sample coordinates
    const real nsx = (sx + _ratio * 0.5_r) / _ratio;
    const real nsy = sy + 0.5_r;
    return Vec2u(static_cast<unsigned>(round(nsx * w)), static_cast<unsigned>(round(nsy * h)));
}

Vec2u
pixel_index(const Vec2& _res, const Vec2& sample_pos) noexcept
{
    return pixel_index(_res, ratio(_res), sample_pos);
}

constexpr real
pixel_width(const Vec2& _res) noexcept
{
    const real w = _res.template get<0>();
    return 1_r / w;
}

constexpr real
pixel_width(const Vec2u& _res) noexcept
{
    const real w = static_cast<real>(_res.template get<0>());
    return 1_r / w;
}

template <typename Image>
struct Film
{
    Image img;
    Vec2  res;
    real  _ratio;
    real  _pixel_width;
    // radius of pixel bounding circle
    real _pixel_radius;

    explicit Film(const Vec2u& res_u) noexcept :
        img(res_u), res(res_u), _ratio(ratio(res)), _pixel_width(pixel_width(res)),
        _pixel_radius(sqrt(2_r * _pixel_width * _pixel_width))
    {
    }

    Vec2
    sample_space(const Vec2u& _pidx) const noexcept
    {
        CHECK_INDEX(get_x(_pidx), get_x(img.res()));
        CHECK_INDEX(get_y(_pidx), get_y(img.res()));
        return lucid::sample_space(res, _ratio, _pidx);
    }

    Vec2u
    pixel_index(const Vec2& pos) const noexcept
    {
        // we require posx to be in range [-ratio/2, ratio/2]
        // posy in range [-0.5, 0.5]
        ASSERT(inclusive_range(-_ratio * 0.5_r, _ratio * 0.5_r)(pos.template get<0>()),
               "Invalid X sample position");
        ASSERT(inclusive_range(-0.5_r, 0.5_r)(pos.template get<1>()), "Invalid Y sample position");

        return lucid::pixel_index(res, _ratio, pos);
    }
};
} // namespace lucid
