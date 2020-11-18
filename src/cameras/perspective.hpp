// -*- C++ -*-
// perspective.hpp --

#pragma once

#include <ray_traversal/ray_traversal.hpp>

namespace lucid
{
real
convert_fov(const real fov) noexcept
{
    return tan(fov * 0.5_r);
}

real
invert_fov(const real f) noexcept
{
    return atan(f) * 2_r;
}

struct perspective
{
    struct shoot
    {
        Vec2 tan_fov_half{1_r};
        Mat4 transform{Mat4::identity()};

        constexpr shoot() noexcept {}

        template <template <typename, size_t> typename Container>
        constexpr shoot(const real cfovy, const Mat4_<Container>& _transform) noexcept :
            tan_fov_half(cfovy), transform(_transform)
        {
        }

        template <template <typename, size_t> typename Container>
        constexpr Ray
        operator()(const Vec2_<Container>& ndc) const noexcept
        {
            return apply_transform(transform, Ray(Vec3(0_r), make_normal(ndc * tan_fov_half, 1_r)));
        }
    };
};
} // namespace lucid
