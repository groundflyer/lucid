// -*- C++ -*-
// disk.hpp --
//

/// @file
/// Definition of disk primitive.

#pragma once

#include "aabb.hpp"

namespace lucid
{
/// @brief Disk.
template <template <typename, size_t> typename Container>
struct Disk_
{
    Vec3_<Container> position;
    Vec3_<Container> normal;
    real             radius;

    constexpr Disk_() noexcept {}

    template <template <typename, size_t> typename Container1,
              template <typename, size_t>
              typename Container2>
    constexpr Disk_(const Vec3_<Container1>& _position,
                    const Vec3_<Container2>& _normal,
                    const real               _radius) noexcept :
        position(_position),
        normal(normalize(_normal)), radius(_radius)
    {
    }
};

template <template <typename, size_t> typename Container>
Disk_(const Vec3_<Container>&, const Vec3_<Container>&, const real) -> Disk_<Container>;

using Disk = Disk_<std::array>;

namespace prim_fn
{
/// @brief Compute ray-disk intersection.
template <template <typename, size_t> typename DiskContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Disk_<DiskContainer>& prim) noexcept
{
    const auto& [o, d]    = ray;
    const auto& [p, n, r] = prim;
    const real t          = dot(p - o, n) / dot(d, n);
    return Intersection(!std::signbit(t) && distance(o + d * t, p) < r, t, Vec2{});
}

/// @brief Return disk normal.
template <template <typename, size_t> typename DiskContainer,
          template <typename, size_t>
          typename PosContainer>
constexpr Vec3
normal(const Vec3_<PosContainer>&, const Disk_<DiskContainer>& prim) noexcept
{
    return prim.normal;
}

/// @brief Sample a point on disk surface.
template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Vec3
sample(const Vec2_<SContainer>& s, const Disk_<PContainer>& prim) noexcept
{
    const auto& [p, zaxis, r] = prim;
    const auto& [t1, t2]      = s;
    const real sr             = r * math::sqrt(t1);
    const real theta          = 2_r * Pi * t2;
    const Vec3 point{sr * math::sin(theta), sr * math::cos(theta), 0_r};
    return dot(basis_matrix(zaxis), point) + p;
}

/// @brief Return disk position.
template <template <typename, size_t> typename Container>
constexpr Vec3
centroid(const Disk_<Container>& prim) noexcept
{
    return prim.position;
}

/// @brief Compute axis-aligned box bounding a disk.
template <template <typename, size_t> typename Container>
constexpr AABB
bound(const Disk_<Container>& prim) noexcept
{
    const auto& [p, n, r] = prim;
    const real h          = math::abs(math::sqrt(1_r - pow<2>(dot(n, Vec3(0, 1, 0)))) * r);
    const Vec3 offset(r, h, r);
    return AABB{p - offset, p + offset};
}

/// @brief Transform disk with a transformation matrix.
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename PrimContainer>
constexpr Disk
apply_transform(const Mat4_<MatContainer>& t, const Disk_<PrimContainer>& prim) noexcept
{
    const auto& [p, n, r] = prim;
    return Disk(apply_transform_p(t, p), apply_transform_n(t, n), r);
}
} // namespace prim_fn
} // namespace lucid
