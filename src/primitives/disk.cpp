// -*- C++ -*-
// disk.cpp -- 

#include "disk.hpp"


namespace yapt
{
    Disk::Disk() {}

    Disk::Disk(const Vec3 & p, const Vec3 & n, const real & r) : _p(p), _n(n)
    { _r2 = r * r; }

    Intersection
    Disk::intersect(const Ray & ray,
		    const real & t_min, const real & t_max) const noexcept
    {
	ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);

	const auto t = intersect_plane(ray, _p, _n);

	bool intersected = false;

	if (std::isnormal(t))
	    intersected =
		length2((ray.origin() + ray.dir() * t) - _p) <= _r2 &&
		in_range(t, t_min, t_max);

	return Intersection(intersected, t, ray, this);
    }

    Vec3
    Disk::normal(const Intersection &) const noexcept
    {
	return _n;
    }

    Vec3
    Disk::tangent(const Intersection &) const noexcept
    {
	Vec3 tmp = _n ^ Vec3(1, 0, 0);
	return _n ^ tmp;
    }
}
