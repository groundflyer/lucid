// -*- C++ -*-
// plane.hpp --
//

#pragma once

#include "primitive.hpp"


namespace yapt
{
    // also used in disk
    constexpr real
    intersect_plane(const Ray & ray, const Vec3 & p, const Vec3 & n)
    { return ((p - ray.origin()) % n) / (ray.dir() % n); }

    // an infinite plane
    class Plane : public Primitive
    {
	Vec3 _p;		// position
	Vec3 _n;		// normal
    public:
	Plane();

	Plane(const Vec3 & p, const Vec3 & n);

	Intersection
	intersect(const Ray & ray,
		  const real & t_min, const real & t_max) const noexcept;

	Vec3
	normal(const Intersection & isect) const noexcept;
    };
}
