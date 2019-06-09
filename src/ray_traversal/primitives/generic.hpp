// -*- C++ -*-
// primitive.hpp
//

#pragma once

#include "aabb.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "quad.hpp"
#include "disk.hpp"

#include <variant>


namespace lucid
{
    template <template <typename, size_t> typename Container>
    using GenericPrimitive_ = std::variant<AABB_<Container>,
                                           Sphere_<Container>,
                                           Triangle_<Container>,
                                           Quad_<Container>,
                                           Disk_<Container>>;

    using GenericPrimitive = std::variant<AABB, Sphere, Triangle, Quad, Disk>;


    template <template <typename, size_t> typename PrimContainer,
			  template <typename, size_t> typename RayContainer>
    constexpr Intersection
    intersect(const Ray_<RayContainer>& ray,
              const GenericPrimitive_<PrimContainer>& prim) noexcept
    {
        return std::visit([&](const auto& held_prim)
                          { return intersect(ray, held_prim); },
                          prim);
    }

	template <template <typename, size_t> typename PrimContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr Normal
    normal(const Ray_<RayContainer>& ray,
           const Intersection_<IsectContainer>& isect,
           const GenericPrimitive_<PrimContainer>& prim) noexcept
    {
        return std::visit([&](const auto& held_prim)
                          { return normal(ray, isect, held_prim); },
            prim);
    }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr Point
    sample(const Vec2_<SContainer>& s,
           const GenericPrimitive_<PContainer>& prim) noexcept
    {
        return std::visit([&](const auto& held_prim)
                          { return sample(s, held_prim); },
            prim);
    }

    template <template <typename, size_t> typename Container>
    constexpr Point
    centroid(const GenericPrimitive_<Container>& prim) noexcept
    {
        return std::visit([](const auto& held_prim)
                          { return centroid(held_prim); },
            prim);
    }

    template <template <typename, size_t> typename Container>
    constexpr AABB
    bound(const GenericPrimitive_<Container>& prim) noexcept
    {
        return std::visit([](const auto& held_prim)
                          { return bound(held_prim); },
            prim);
    }

    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename PrimContainer>
    constexpr GenericPrimitive
    apply_transform(const Mat4_<MatContainer>& t,
                    const GenericPrimitive_<PrimContainer>& prim) noexcept
    {
        return std::visit([&t](const auto& held_prim)
                          { return GenericPrimitive(apply_transform(t, held_prim)); },
            prim);
    }
}
