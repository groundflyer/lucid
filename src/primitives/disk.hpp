// -*- C++ -*-
// disk.hpp --
//

#pragma once

#include "plane.hpp"


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Disk_ : public Plane_<Container>
    {
        real radius2;

        constexpr
        Disk_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2>
        constexpr
        Disk_(const Point_<Container1>& _position,
              const Normal_<Container2>& _normal,
              const real _radius) :
        Plane_<Container>(_position, _normal),
            radius2(_radius * _radius)
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
              const Disk_<DiskContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
    {
        auto plane_isect = intersect(ray, static_cast<Plane_<DiskContainer>>(prim), range);
        const auto& [o, d] = ray;
        return Intersection(plane_isect && length2((o + d * plane_isect.distance()) - prim.position) <= prim.radius2,
                            plane_isect.distance(),
                            plane_isect.coords());
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
