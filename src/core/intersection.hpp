// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "ray.hpp"


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Intersection_
    {
		const bool intersect = false;
		const real t = -1_r;
		const Vec2_<Container> st {};	// parametric coordinates

		constexpr
		Intersection_() {}

		template <template <typename, size_t> typename Container2>
		constexpr
		Intersection_(const bool _i,
					  const real& _t,
					  const Vec2_<Container2>& _st) :
			intersect(_i),
			t(_t),
			st(_st)
		{}

		constexpr
		operator bool() const noexcept
		{ return intersect; }
    };

	template <template <typename, size_t> typename Container>
	Intersection_(const bool,
				  const real&,
				  const Vec2_<Container>&) -> Intersection_<Container>;

	using Intersection = Intersection_<std::array>;
}
