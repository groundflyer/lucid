// -*- C++ -*-
// plane.cpp -- 

#include "plane.hpp"


namespace yapt
{
    Plane::Plane() {}

    Plane::Plane(const Vec3 & p, const Vec3 & n) : _p(p), _n(n) {}

    Intersection
    Plane::intersect(const Ray & ray,
		     const real & t_min, const real & t_max) const noexcept
    {
	ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);

	const auto t = intersect_plane(ray, _p, _n);

	bool intersected = false;

	if (std::isnormal(t))
	    intersected = in_range(t, t_min, t_max);

	return Intersection(intersected, t, ray, this);
    }

    Vec3
    Plane::normal(const Intersection &) const noexcept
    {
	return _n;
    }
}
