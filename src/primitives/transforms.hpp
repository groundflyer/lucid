// -*- C++ -*-
// transforms.hpp
// Primitives apply_transform declarations for use in base/transforms.hpp

#pragma once

#include <base/types.hpp>
#include <utils/typelist.hpp>

namespace lucid
{
template <template <typename, std::size_t> typename Container>
struct AABB_;
template <template <typename, std::size_t> typename Container>
struct Disk_;
template <template <typename, size_t> typename Container>
using Quad_ = std::array<Vec3_<Container>, 4>;
template <template <typename, size_t> typename Container>
using Triangle_ = std::array<Vec3_<Container>, 3>;
template <template <typename, size_t> typename Container>
struct Sphere_;
template <template <typename, size_t> typename Container>
using AllPrimTypes = typelist<AABB_<Container>,
                              Sphere_<Container>,
                              Triangle_<Container>,
                              Quad_<Container>,
                              Disk_<Container>>;

template <template <typename, size_t> typename Container>
using GenericPrimitive_ = typename AllPrimTypes<Container>::variant;

using GenericPrimitive = GenericPrimitive_<std::array>;

using AABB     = AABB_<std::array>;
using Disk     = Disk_<std::array>;
using Triangle = Triangle_<std::array>;
using Quad     = Quad_<std::array>;
using Sphere   = Sphere_<std::array>;

namespace fn
{
template <template <typename, std::size_t> typename MatContainer,
          template <typename, std::size_t>
          typename PrimContainer>
constexpr AABB
apply_transform(const Mat4_<MatContainer>&, const AABB_<PrimContainer>&) noexcept;

template <template <typename, std::size_t> typename MatContainer,
          template <typename, std::size_t>
          typename PrimContainer>
constexpr Disk
apply_transform(const Mat4_<MatContainer>&, const Disk_<PrimContainer>&) noexcept;
template <template <typename, std::size_t> typename MatContainer,
          template <typename, std::size_t>
          typename TriangleContainer>
constexpr Triangle
apply_transform(const Mat4_<MatContainer>&, const Triangle_<TriangleContainer>&) noexcept;
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename QuadContainer>
constexpr Quad
apply_transform(const Mat4_<MatContainer>&, const Quad_<QuadContainer>&) noexcept;

template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename PrimContainer>
constexpr Sphere
apply_transform(const Mat4_<MatContainer>&, const Sphere_<PrimContainer>&) noexcept;
template <template <typename, size_t> typename MatContainer, typename... Prims>
constexpr std::variant<Prims...>
apply_transform(const Mat4_<MatContainer>& t, const std::variant<Prims...>& prim) noexcept;
} // namespace fn
} // namespace lucid
