// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "types.hpp"

namespace lucid
{
    template <template <typename, size_t> typename Container>
    struct Intersection_
    {
        bool intersect;
        real t;
        Vec2_<Container> st;

        constexpr
        Intersection_() : intersect(false),
                          t(std::numeric_limits<real>::max()),
                          st(Vec2(0))
        {}

		template <template <typename, size_t> typename Container2>
		constexpr
		Intersection_(const bool _intersect,
                      const real& _t,
                      const Vec2_<Container2>& _st) :
            intersect(_intersect),
            t(intersect ? _t : std::numeric_limits<real>::max()),
            st(_st) {}

        constexpr
        operator bool() const noexcept
        { return intersect; }
    };

	template <template <typename, size_t> typename Container>
	Intersection_(const bool, const real&,
                  const Vec2_<Container>&) -> Intersection_<Container>;

    using Intersection = Intersection_<std::array>;
}
