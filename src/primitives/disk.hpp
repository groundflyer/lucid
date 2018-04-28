// -*- C++ -*-
// disk.hpp --
//

#pragma once

#include "plane.hpp"


namespace yapt
{
    template <template <typename, size_t> typename PointContainer,
              template <typename, size_t> typename NormalContainer>
    struct Disk_ : public Plane_<PointContainer, NormalContainer>
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
        Plane_<PointContainer, NormalContainer>(_position, _normal),
            radius2(_radius * _radius)
        {}
    };


    template <template <typename, size_t> typename Container>
    Disk_(const Point_<Container>&,
          const Normal_<Container>&,
          const real) -> Disk_<Container, Container>;

    using Disk = Disk_<std::array, std::array>;


	template <template <typename, size_t> typename DiskPContainer,
              template <typename, size_t> typename DiskNContainer,
			  template <typename, size_t> typename RayPContainer,
			  template <typename, size_t> typename RayNContainer>
	constexpr auto
	intersect(const Ray_<RayPContainer, RayNContainer>& ray,
              const Disk_<DiskPContainer, DiskNContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
    {
        auto ret = intersect(ray, static_cast<Plane_<DiskPContainer, DiskNContainer>>(prim), range);

        const auto& [o, d] = ray;
        ret.intersect &= length2((o + d * ret.t) - prim.position) <= prim.radius2;
        return ret;
    }
}
