// -*- C++ -*-
// plane.hpp --
//

#pragma once

#include "intersection.hpp"
#include "ray.hpp"
#include <base/range.hpp>

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
              const Plane_<PlaneContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [p, n] = prim;
        const auto t = ((p - o).dot(n) / (d.dot(n)));
        return Intersection{t > 0_r, t, Vec2{}};
    }

	template <template <typename, size_t> typename PlaneContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    compute_normal(const Ray_<RayContainer>&,
                   const Intersection_<IsectContainer>&,
                   const Plane_<PlaneContainer>& prim) noexcept
    { return Normal(prim.normal); }
}
