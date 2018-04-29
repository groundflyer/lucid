// -*- C++ -*-
// plane.hpp --
//

#pragma once

#include <core/intersection.hpp>
#include <core/range.hpp>
#include <core/ray.hpp>

namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Plane_
    {
        Point_<Container> position;
        Normal_<Container> normal;

        constexpr
        Plane_ () {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2>
        constexpr
        Plane_(const Point_<Container1>& _position,
               const Normal_<Container2>& _normal) :
        position(_position),
            normal(_normal)
        {}
    };


    template <template <typename, size_t> typename Container>
    Plane_(const Point_<Container>&,
           const Normal_<Container>&) -> Plane_<Container>;

    using Plane = Plane_<std::array>;


	template <template <typename, size_t> typename PlaneContainer,
			  template <typename, size_t> typename RayContainer>
	constexpr auto
	intersect(const Ray_<RayContainer>& ray,
              const Plane_<PlaneContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [p, n] = prim;
        const auto t = ((p - o).dot(n) / (d.dot(n)));

        return Intersection(range.encloses(t), t, Vec2(0));
    }
}
