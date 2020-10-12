// -*- C++ -*-
// sphere.hpp --
//

#pragma once

#include "aabb.hpp"

#include <iostream>

namespace lucid
{
template <template <typename, size_t> typename Container>
struct Sphere_
{
    Vec3_<Container> center;
    real             radius;

    constexpr Sphere_() noexcept {}

    template <template <typename, size_t> typename Container1>
    constexpr Sphere_(const Vec3_<Container1>& _center, const real& _radius) noexcept :
        center(_center), radius(_radius)
    {
    }
};

template <template <typename, size_t> typename Container>
Sphere_(const Vec3_<Container>&, const real&) -> Sphere_<Container>;

using Sphere = Sphere_<std::array>;

template <template <typename, size_t> typename SphereContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Sphere_<SphereContainer>& prim) noexcept
{
    const auto& [o, d]  = ray;
    const real a        = dot(d, d);
    const Vec3 pc       = o - prim.center;
    const real b        = dot(d, pc * 2_r);
    const real c        = dot(pc, pc) - pow<2>(prim.radius);
    const auto [hit, t] = quadratic(a, b, c);
    return Intersection{hit, t, Vec2{}};
}

template <template <typename, size_t> typename SphereContainer,
          template <typename, size_t>
          typename RayContainer,
          template <typename, size_t>
          typename IsectContainer>
constexpr Vec3
normal(const Ray_<RayContainer>&            ray,
       const Intersection_<IsectContainer>& isect,
       const Sphere_<SphereContainer>&      prim) noexcept
{
    const auto& [o, d] = ray;
    const Vec3 pos     = o + d * isect.t;
    return normalize(pos - prim.center);
}

template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Vec3
sample(const Vec2_<SContainer>& s, const Sphere_<PContainer>& prim) noexcept
{
    const auto& [c, r]   = prim;
    const auto& [t1, t2] = s;
    const real u         = 2_r * t1 - 1;
    const real theta     = 2_r * Pi * t2;
    const real _u        = math::sqrt(1_r - pow<2>(u));
    return Vec3(_u * math::cos(theta), _u * math::sin(theta), u) * r + c;
}

template <template <typename, size_t> typename Container>
constexpr Vec3
centroid(const Sphere_<Container>& prim) noexcept
{
    return prim.center;
}

template <template <typename, size_t> typename Container>
constexpr AABB
bound(const Sphere_<Container>& prim) noexcept
{
    const auto& [c, r] = prim;
    return AABB{c - r, c + r};
}

template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename PrimContainer>
constexpr Sphere
apply_transform(const Mat4_<MatContainer>& t, const Sphere_<PrimContainer>& prim) noexcept
{
    const auto& [c, r] = prim;
    return Sphere(apply_transform_p(t, c), r);
}
} // namespace lucid
