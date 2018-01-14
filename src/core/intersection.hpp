// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "ray.hpp"

#include <optional>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Intersection_
    {
		const bool intersect = false;
		const std::optional<real> t;
		const std::optional<size_t> primid;
		const std::optional<Point_<Container>> pos;
		const std::optional<Vec2_<Container>> st;	// parametric coordinates
		const std::optional<Normal_<Container>> wi;	// incident direction

		constexpr
		Intersection_() = delete

		template <template <typename, size_t> typename RayContainer,
				  template <typename, size_t> typename Vec2Container>
		constexpr
		Intersection_(const real arg_t,
					  const Ray_<RayContainer>& ray,
					  const std::optional<size_t> arg_primid = {},
					  const std::optional<Vec2_<Vec2Container>> & arg_st = {}) :
			intersect(true),
			t(arg_t),
			primid(arg_primid),
			pos(ray.origin+ray.dir*t),
			st(arg_st),
			wi(ray.dir) {}

		constexpr
		operator bool() const noexcept
		{ return intersect; }
    };

	template <template <typename, size_t> typename Container>
	Intersection_(const real,
				  const Ray_<Container>&,
				  const std::optional<size_t>,
				  const std::optional<Vec2_<Container>>) -> Intersection_<Container>;

	using Intersection = Intersection_<StaticContainer>;
}
