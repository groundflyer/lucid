// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "ray.hpp"
#include "primitive.hpp"
#include <limits>


namespace yapt
{
    class Primitive;

    template <template <typename, size_t> class Container>
    class Intersection
    {
	bool _intersect = false;
	real _t = std::numeric_limits<real>::infinity(); // distance
	Vec3<Container> _pos;
	Vec2<Container> _st; 		// parametric coordinates
	Primitive const * hitprim = nullptr;

    public:
	constexpr
	Intersection() {}

	explicit constexpr
	Intersection(const bool i) : _intersect(i) {}

	template <template <typename, size_t> class Container1>
	constexpr
	Intersection(const bool i,
		     const real & t,
		     const Ray<Container1> & ray) : _intersect(i), _t(t), _pos(ray.origin()+ray.dir()*t) {}

	template <
	    template <typename, size_t> class Container1,
	    template <typename, size_t> class Container2>
	constexpr
	Intersection(const bool i,
		     const real & t,
		     const Ray<Container1> & ray,
		     const Primitive<Container2> * prim)
	    : _intersect(i), _t(t), _pos(ray.origin()+ray.dir()*t), hitprim(prim) {}

	template <
	    template <typename, size_t> class Container1,
	    template <typename, size_t> class Container2,
	    template <typename, size_t> class Container3>
	constexpr
	Intersection(const bool i,
		     const real & t,
		     const Ray<Container1> & ray,
		     const Primitive<Container2> * prim,
		     const Vec2<Container3> & st)
	    : _intersect(i), _t(t), _pos(ray.origin()+ray.dir()*t), _st(st), hitprim(prim) {}

	constexpr
	operator bool() const noexcept
	{ return _intersect; }

	constexpr const Vec2<Container>&
	st() const noexcept
	{ return _st; }

	constexpr real
	t() const noexcept
	{ return _t; }

	constexpr const Vec3<Container>&
	pos() const noexcept
	{ return _pos; }

	constexpr const Primitive &
	prim() const noexcept
	{ return *hitprim; }
    };
}
