// -*- C++ -*-
// plane.hpp --
//

#pragma once

#include "primitive.hpp"


namespace yapt
{
    constexpr real
    intersect_plane(const Ray & ray, const Vec3 & p, const Vec3 n) noexcept
    { return ((p - ray.origin()) % n) / (ray.dir() % n); }


    // an infinite plane
    class Plane : public Primitive
    {
	Vec3 _p;		// position
	Vec3 _n;		// normal
    public:
	constexpr
	Plane() {}

	constexpr
	Plane(const Vec3 & p, const Vec3 & n) : _p(p), _n(n) {}

	constexpr Intersection
	intersect(const Ray & ray,
		  const real t_min, const real t_max) const noexcept
	{
	    ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);

	    const auto t = intersect_plane(ray, _p, _n);

	    bool intersected = false;

	    if (std::isnormal(t))
		intersected = in_range(t, t_min, t_max);

	    return Intersection(intersected, t, _n);
	}
    };
}
