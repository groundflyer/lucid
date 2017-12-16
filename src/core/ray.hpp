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
	const Point<Container> _origin;
	const Normal<Container> _dir;
    public:
	constexpr
	Ray() = delete;

	template <
	    template <typename, size_t> class Container1,
	    template <typename, size_t> class Container2>
	constexpr
	Ray(const Point<Container1> & origin,
	    const Normal<Container2> & dir) : _origin(origin), _dir(dir) {}

	constexpr const Point<Container>&
	origin() const noexcept
	{ return _origin; }

	constexpr const Normal<Container>&
	dir() const noexcept
	{ return _dir; }
    };
}
