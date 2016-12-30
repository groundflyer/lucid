// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "ray.hpp"
#include "primitive.hpp"


namespace yapt
{
    // forward declaration of Primitive
    class Primitive;

    class Intersection
    {
	bool _intersect = false;
	real _t = 0;		// distance
	Vec3 _pos;
	Vec2 _st; 		// parametric coordinates
	Primitive const * hitprim = nullptr;

    public:
	constexpr
	Intersection() {}

	explicit constexpr
	Intersection(const bool i) : _intersect(i) {}

	constexpr
	Intersection(const bool i,
		     const real & t,
		     const Ray & ray) : _intersect(i), _t(t), _pos(ray.origin()+ray.dir()*t) { }

	constexpr
	Intersection(const bool i,
		     const real & t,
		     const Ray & ray,
		     const Primitive * prim)
	    : _intersect(i), _t(t), _pos(ray.origin()+ray.dir()*t), hitprim(prim) { }

	constexpr
	Intersection(const bool i,
		     const real & t,
		     const Ray & ray,
		     const Primitive * prim,
		     const Vec2 & st)
	    : _intersect(i), _t(t), _pos(ray.origin()+ray.dir()*t), _st(st), hitprim(prim) { }

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
	pos() const noexcept
	{ return _pos; }

	const constexpr Primitive *
	prim() const noexcept
	{ return hitprim; }
    };
}
