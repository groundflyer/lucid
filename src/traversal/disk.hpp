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
        real radius2;

        constexpr
        Disk_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2>
        constexpr
        Disk_(const Point_<Container1>& _position,
              const Normal_<Container2>& _normal,
              const real _radius) :
            plane(_position, _normal),
            radius2(math::pow<2>(_radius))
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
        auto plane_isect = intersect(ray, prim.plane);
        const auto& [o, d] = ray;
        plane_isect.intersect &= length2((o + d * plane_isect.t) - prim.plane.position) <= prim.radius2;
        return plane_isect;
    }

	template <template <typename, size_t> typename DiskContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    compute_normal(const Ray_<RayContainer>& ray,
                   const Intersection_<IsectContainer>& isect,
                   const Disk_<DiskContainer>& prim) noexcept
    { return compute_normal(ray, isect, static_cast<Plane_<DiskContainer>>(prim)); }
}
