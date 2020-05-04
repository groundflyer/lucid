// -*- C++ -*-
// disk.hpp --
//

#pragma once

#include "aabb.hpp"

namespace lucid
{
template <template <typename, size_t> typename Container>
struct Disk_
{
    Point_<Container>  position;
    Normal_<Container> normal;
    real               radius;

    constexpr Disk_() noexcept {}

    template <template <typename, size_t> typename Container1,
              template <typename, size_t>
              typename Container2>
    constexpr Disk_(const Point_<Container1>&  _position,
                    const Normal_<Container2>& _normal,
                    const real                 _radius) noexcept :
        position(_position),
        normal(_normal), radius(_radius)
    {
    }
};

template <template <typename, size_t> typename Container>
Disk_(const Point_<Container>&, const Normal_<Container>&, const real)->Disk_<Container>;

using Disk = Disk_<std::array>;

template <template <typename, size_t> typename DiskContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Disk_<DiskContainer>& prim) noexcept
{
    const auto& [o, d]    = ray;
    const auto& [p, n, r] = prim;
    const real t          = ((p - o).dot(n) / (d.dot(n)));
    return Intersection(!std::signbit(t) && distance(o + d * t, p) < r, t, Vec2{});
}

template <template <typename, size_t> typename DiskContainer,
          template <typename, size_t>
          typename RayContainer,
          template <typename, size_t>
          typename IsectContainer>
constexpr Normal
normal(const Ray_<RayContainer>&,
       const Intersection_<IsectContainer>&,
       const Disk_<DiskContainer>& prim) noexcept
{
    return prim.normal;
}

template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Point
sample(const Vec2_<SContainer>& s, const Disk_<PContainer>& prim) noexcept
{
    const auto& [p, zaxis, r] = prim;
    const auto& [t1, t2]      = s;
    const real  sr            = r * math::sqrt(t1);
    const real  theta         = 2_r * Pi * t2;
    const Point point{sr * math::sin(theta), sr * math::cos(theta), 0_r};
    return Point(basis_matrix(zaxis).dot(point) + p);
}

template <template <typename, size_t> typename Container>
constexpr Point
centroid(const Disk_<Container>& prim) noexcept
{
    return prim.position;
}

template <template <typename, size_t> typename Container>
constexpr AABB
bound(const Disk_<Container>& prim) noexcept
{
    const auto& [p, n, r] = prim;
    const real  h         = math::abs(math::sqrt(1_r - pow<2>(n.dot(Vec3(0, 1, 0)))) * r);
    const Point offset(r, h, r);
    return AABB{p - offset, p + offset};
}

template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename PrimContainer>
constexpr Disk
apply_transform(const Mat4_<MatContainer>& t, const Disk_<PrimContainer>& prim) noexcept
{
    const auto& [p, n, r] = prim;
    return Disk(apply_transform(t, p), apply_transform(t, n), r);
}
} // namespace lucid
