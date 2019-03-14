// -*- C++ -*-
// disk.hpp --
//

#pragma once

#include "plane.hpp"


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Disk_
    {
        Plane_<Container> plane;
        real radius;

        constexpr
        Disk_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2>
        constexpr
        Disk_(const Point_<Container1>& _position,
              const Normal_<Container2>& _normal,
              const real _radius) :
            plane(_position, _normal),
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
	constexpr auto
	intersect(const Ray_<RayContainer>& ray,
              const Disk_<DiskContainer>& prim) noexcept
    {
        const auto& plane = prim.plane;
        auto plane_isect = intersect(ray, plane);
        const auto& [o, d] = ray;
        plane_isect.intersect &= length((o + d * plane_isect.t) - plane.position) <= prim.radius;
        return plane_isect;
    }

	template <template <typename, size_t> typename DiskContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    normal(const Ray_<RayContainer>& ray,
           const Intersection_<IsectContainer>& isect,
           const Disk_<DiskContainer>& prim) noexcept
    { return normal(ray, isect, prim.plane); }

    template <template <typename, size_t> typename Container,
              typename Generator>
    constexpr auto
    sample(Generator&& gen,
           const Disk_<Container>& prim) noexcept
    {
        const auto& [pos, zaxis] = prim.plane;
        const auto sr = prim.radius * math::sqrt(gen());
        const auto theta = 2_r * math::PI<real> * gen();
        const Vec3 point{sr * math::sin(theta), sr * math::cos(theta), 0_r};
        const auto [xaxis, yaxis] = basis(zaxis);
        return transpose(Mat3(xaxis, yaxis, zaxis)).dot(point) + pos;
    }
}
