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
    Point_<Container> center;
    real              radius;

    constexpr Sphere_() noexcept {}

    template <template <typename, size_t> typename Container1>
    constexpr Sphere_(const Point_<Container1>& _center, const real& _radius) noexcept :
        center(_center), radius(_radius)
    {
    }
};

template <template <typename, size_t> typename Container>
Sphere_(const Point_<Container>&, const real&)->Sphere_<Container>;

using Sphere = Sphere_<std::array>;

template <template <typename, size_t> typename SphereContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Sphere_<SphereContainer>& prim) noexcept
{
    const auto& [o, d]  = ray;
    const real  a       = d.dot(d);
    const Point pc      = o - prim.center;
    const real  b       = d.dot(pc * 2_r);
    const real  c       = pc.dot(pc) - pow<2>(prim.radius);
    const auto [hit, t] = quadratic(a, b, c);
    return Intersection{hit, t, Vec2{}};
}

template <template <typename, size_t> typename SphereContainer,
          template <typename, size_t>
          typename RayContainer,
          template <typename, size_t>
          typename IsectContainer>
constexpr Normal
normal(const Ray_<RayContainer>&            ray,
       const Intersection_<IsectContainer>& isect,
       const Sphere_<SphereContainer>&      prim) noexcept
{
    const auto& [o, d] = ray;
    const Point pos    = o + d * isect.t;
    return Normal(pos - prim.center);
}

template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Point
sample(const Vec2_<SContainer>& s, const Sphere_<PContainer>& prim) noexcept
{
    const auto& [c, r]   = prim;
    const auto& [t1, t2] = s;
    const real u         = 2_r * t1 - 1;
    const real theta     = 2_r * Pi * t2;
    const real _u        = math::sqrt(1_r - pow<2>(u));
    return Point(_u * math::cos(theta), _u * math::sin(theta), u) * r + c;
}

template <template <typename, size_t> typename Container>
constexpr Point
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
    return Sphere(apply_transform(t, c), r);
}
} // namespace lucid
