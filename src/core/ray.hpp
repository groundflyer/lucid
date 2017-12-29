// -*- C++ -*-
// ray.hpp --
//

#pragma once

#include "types.hpp"


namespace yapt
{
    template <template <typename, size_t> class Container>
    class Ray
    {
		const Point<Container> m_origin;
		const Normal<Container> m_dir;
    public:
		constexpr
		Ray() = delete;

		template <
			template <typename, size_t> class Container1,
			template <typename, size_t> class Container2>
		constexpr
		Ray(const Point<Container1> & origin,
			const Normal<Container2> & dir) : m_origin(origin), m_dir(dir) {}

		constexpr const Point<Container>&
		origin() const noexcept
		{ return m_origin; }

		constexpr const Normal<Container>&
		dir() const noexcept
		{ return m_dir; }
    };
}
