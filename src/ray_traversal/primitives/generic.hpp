// -*- C++ -*-
// primitive.hpp
//

#pragma once

#include "aabb.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "quad.hpp"
#include "disk.hpp"

#include <utils/typelist.hpp>


namespace lucid
{
    template <template <typename, size_t> typename Container>
    using AllPrimTypes = typelist<AABB_<Container>,
                                  Sphere_<Container>,
                                  Triangle_<Container>,
                                  Quad_<Container>,
                                  Disk_<Container>>;

    template <template <typename, size_t> typename Container>
    using GenericPrimitive_ = typename AllPrimTypes<Container>::variant;

    using GenericPrimitive = GenericPrimitive_<std::array>;


    template <template <typename, size_t> typename RayContainer,
              typename ... Prims>
    constexpr Intersection
    intersect(const Ray_<RayContainer>& ray,
              const std::variant<Prims...>& prim) noexcept
    {
        return std::visit([&](const auto& held_prim){ return intersect(ray, held_prim); },
                          prim);
    }

	template <template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer,
              typename ... Prims>
    constexpr Normal
    normal(const Ray_<RayContainer>& ray,
           const Intersection_<IsectContainer>& isect,
           const std::variant<Prims...>& prim) noexcept
    {
        return std::visit([&](const auto& held_prim){ return normal(ray, isect, held_prim); },
                          prim);
    }

	template <template <typename, size_t> typename SContainer,
              typename ... Prims>
    constexpr Point
    sample(const Vec2_<SContainer>& s,
           const std::variant<Prims...>& prim) noexcept
    {
        return std::visit([&](const auto& held_prim){ return sample(s, held_prim); },
                          prim);
    }

    template <typename ... Prims>
    constexpr Point
    centroid(const std::variant<Prims...>& prim) noexcept
    {
        return std::visit([](const auto& held_prim){ return centroid(held_prim); },
                          prim);
    }

    template <typename ... Prims>
    constexpr AABB
    bound(const std::variant<Prims...>& prim) noexcept
    {
        return std::visit([](const auto& held_prim){ return bound(held_prim); },
                          prim);
    }

    template <template <typename, size_t> typename MatContainer,
			  typename ... Prims>
    constexpr auto
    apply_transform(const Mat4_<MatContainer>& t,
                    const std::variant<Prims...>& prim) noexcept
    {
        return std::visit([&t](const auto& held_prim){ return std::variant<Prims...>(apply_transform(t, held_prim)); },
                          prim);
    }
}
