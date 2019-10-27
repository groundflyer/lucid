// -*- C++ -*-
// perspective.hpp -- 

#pragma once

#include <ray_traversal/ray_traversal.hpp>


namespace lucid::perspective
{
struct shoot
{
    Vec2 tan_fov_half{1_r};
    Mat4 transform{Mat4::identity()};

    constexpr shoot() {}

    template <template <typename, size_t> typename Container>
    constexpr shoot(const real fov, const real aspect_ratio, const Mat4_<Container>& transform) :
        tan_fov_half(math::tan(fov * 0.5_r), math::tan(fov / aspect_ratio * 0.5_r)),
        transform(transform)
    {
    }

    template <template <typename, size_t> typename Container>
    constexpr Ray
    operator()(const Vec2_<Container>& ndc) const noexcept
    {
        const auto& [x, y]       = ndc;
        const auto& [tfhx, tfhy] = tan_fov_half;
        const Normal d(x * tfhx, y * tfhx, 1);
        return apply_transform(transform, Ray(Point(0), d));
    }
};
}
