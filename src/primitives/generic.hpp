// -*- C++ -*-
// primitive.hpp
//

#pragma once

#include "aabb.hpp"
#include "disk.hpp"
#include "quad.hpp"
#include "sphere.hpp"
#include "triangle.hpp"

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

template <template <typename, size_t> typename RayContainer, typename... Prims>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const std::variant<Prims...>& prim) noexcept
{
    return std::visit([&](const auto& held_prim) noexcept { return intersect(ray, held_prim); },
                      prim);
}

template <template <typename, size_t> typename PosContainer, typename... Prims>
constexpr Vec3
normal(const Vec3_<PosContainer>& pos, const std::variant<Prims...>& prim) noexcept
{
    return std::visit([&](const auto& held_prim) noexcept { return normal(pos, held_prim); }, prim);
}

template <template <typename, size_t> typename SContainer, typename... Prims>
constexpr Vec3
sample(const Vec2_<SContainer>& s, const std::variant<Prims...>& prim) noexcept
{
    return std::visit([&](const auto& held_prim) noexcept { return sample(s, held_prim); }, prim);
}

template <typename... Prims>
constexpr Vec3
centroid(const std::variant<Prims...>& prim) noexcept
{
    return std::visit([](const auto& held_prim) noexcept { return centroid(held_prim); }, prim);
}

template <typename... Prims>
constexpr AABB
bound(const std::variant<Prims...>& prim) noexcept
{
    return std::visit([](const auto& held_prim) noexcept { return bound(held_prim); }, prim);
}

template <template <typename, size_t> typename MatContainer, typename... Prims>
constexpr auto
apply_transform(const Mat4_<MatContainer>& t, const std::variant<Prims...>& prim) noexcept
{
    return std::visit(
        [&t](const auto& held_prim) noexcept {
            return std::variant<Prims...>(apply_transform(t, held_prim));
        },
        prim);
}
} // namespace lucid
