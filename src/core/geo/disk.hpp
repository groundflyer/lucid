// -*- C++ -*-
// disk.hpp --
//

#pragma once

#include "plane.hpp"


namespace yapt
{
    class Disk : public Primitive
    {
	Vec3 _p;		// position
	Vec3 _n; 		// normal
	real _r2 = 1;		// square of radius
    public:
	constexpr
	Disk() {}

	constexpr
	Disk(const Vec3 & p, const Vec3 & n, const real r)
	    : _p(p), _n(n)
	{ _r2 = r * r; }

	constexpr Intersection
	intersect(const Ray & ray,
		  const real t_min, const real t_max) const noexcept
	{
	    ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);

	    const auto t = intersect_plane(ray, _p, _n);

	    bool intersected = false;

	    if (std::isnormal(t))
		intersected =
		    length2((ray.origin() + ray.dir() * t) - _p) <= _r2 &&
		    in_range(t, t_min, t_max);

	    return Intersection(intersected, t, _n);
	}
    };
}
