// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "ray.hpp"


namespace yapt
{
    class Intersection
    {
	bool _intersect = false;
	real _t = 0;		// distance
	Vec2 _st; 		// parametric coordinates
	Vec3 _n;		// hit normal

    public:
	constexpr
	Intersection() {}

	explicit constexpr
	Intersection(const bool i) : _intersect(i) {}

	constexpr
	Intersection(const bool i,
		     const real t) : _intersect(i), _t(t) {}

	constexpr
	Intersection(const bool i,
		     const real t,
		     const Vec2 & st) noexcept
	    : _intersect(i), _t(t), _st(st) {}

	constexpr
	Intersection(const bool i,
		     const real t,
		     const Vec3 & n) noexcept
	    : _intersect(i), _t(t), _n(n) {}

	constexpr
	Intersection(const bool i,
		     const real t,
		     const Vec3 & n,
		     const Vec2 & st) noexcept
	    : _intersect(i), _t(t), _st(st), _n(n) {}

	constexpr
	operator bool() const noexcept
	{ return _intersect; }

	constexpr Vec2
	st() const noexcept
	{ return _st; }

	constexpr real
	t() const noexcept
	{ return _t; }

	constexpr Vec3
	n() const noexcept
	{ return _n; }

	constexpr Vec3
	pos(const Ray & ray) const noexcept
	{ return ray.origin() + ray.dir() * _t; }
    };
}
