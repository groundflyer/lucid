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
        Point_<Container> position;
        Normal_<Container> normal;
        real radius;

        constexpr
        Disk_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2>
        constexpr
        Disk_(const Point_<Container1>& _position,
              const Normal_<Container2>& _normal,
              const real _radius) :
            position(_position),
            normal(_normal),
            radius(_radius)
        {}
    };


    template <template <typename, size_t> typename Container>
    Disk_(const Point_<Container>&,
          const Normal_<Container>&,
          const real) -> Disk_<Container>;

    using Disk = Disk_<std::array>;


	template <template <typename, size_t> typename DiskContainer,
			  template <typename, size_t> typename RayContainer>
	constexpr Intersection
	intersect(const Ray_<RayContainer>& ray,
              const Disk_<DiskContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [p, n, r] = prim;
        const auto t = ((p - o).dot(n) / (d.dot(n)));
        return Intersection(t > 0_r && distance(o + d * t, p) < r, t, Vec2{});
    }

	template <template <typename, size_t> typename DiskContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr Normal
    normal(const Ray_<RayContainer>&,
           const Intersection_<IsectContainer>&,
           const Disk_<DiskContainer>& prim) noexcept
    { return prim.normal; }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr Point
    sample(const Vec2_<SContainer>& s,
           const Disk_<PContainer>& prim) noexcept
    {
        const auto& [p, zaxis, r] = prim;
        const auto& [t1, t2] = s;
        const auto sr = r * math::sqrt(t1);
        const auto theta = 2_r * math::PI<real> * t2;
        const Vec3 point{sr * math::sin(theta), sr * math::cos(theta), 0_r};
        const auto [xaxis, yaxis] = basis(zaxis);
        return Point(transpose(Mat3(xaxis, yaxis, zaxis)).dot(point) + p);
    }

    template <template <typename, size_t> typename Container>
    constexpr Point
    centroid(const Disk_<Container>& prim) noexcept
    { return prim.position; }

    template <template <typename, size_t> typename Container>
    constexpr AABB
    bound(const Disk_<Container>& prim) noexcept
    {
        const auto& [p, n, r] = prim;
        const auto h = math::abs(math::sqrt(1_r - pow<2>(n.dot(Vec3(0, 1, 0)))) * r);
        const Vec3 offset(r, h, r);
        return AABB{p - offset, p + offset};
    }
}
