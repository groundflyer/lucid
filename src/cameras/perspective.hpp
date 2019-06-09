// -*- C++ -*-
// perspective.hpp -- 

#pragma once

#include <ray_traversal/ray_traversal.hpp>


namespace lucid::perspective
{
    struct shoot
    {
        real tan_fov_half{1_r};
        Mat4 transform{Mat4::identity()};

        constexpr
        shoot() {}

        template <template <typename, size_t> typename Container>
        constexpr
        shoot(const real& fov,
                          const Mat4_<Container>& transform) :
            tan_fov_half(math::tan(fov * 0.5_r)),
            transform(transform)
        {}

        template <template <typename, size_t> typename Container>
        constexpr Ray
        operator()(const Vec2_<Container>& ndc) const noexcept
        {
            const auto& [x, y] = ndc;
            const Normal d(x * tan_fov_half, y * tan_fov_half, 1);
            return apply_transform(transform, Ray(Point(0), d));
        }
    };
}
