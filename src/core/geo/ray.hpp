// -*- C++ -*-
// ray.hpp --
//

#pragma once

#include <core/basic_types.hpp>


namespace yapt
{
    class Ray
    {
	Vec3 _origin;
	Vec3 _dir;
    public:
	constexpr
	Ray() {}

	constexpr
	Ray(const Vec3 & origin,
	    const Vec3 & dir) : _origin(origin), _dir(dir) { }

	const constexpr Vec3&
	origin() const noexcept
	{ return _origin; }

	const constexpr Vec3&
	dir() const noexcept
	{ return _dir; }

	constexpr Vec3&
	origin() noexcept
	{ return _origin; }

	constexpr Vec3&
	dir() noexcept
	{ return _dir; }
    };
}
