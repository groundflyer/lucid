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
    return Vec2u(static_cast<unsigned>(math::round(nsx * w)),
                 static_cast<unsigned>(math::round(nsy * h)));
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
        _pixel_radius(math::sqrt(2_r * _pixel_width))
    {
    }

    Vec2
    sample_space(const Vec2u& _pidx) const noexcept
    {
        return lucid::sample_space(res, _ratio, _pidx);
    }

    Vec2u
    pixel_index(const Vec2& sample_pos) const noexcept
    {
        return lucid::pixel_index(res, _ratio, sample_pos);
    }
};
} // namespace lucid
