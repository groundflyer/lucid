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
	Disk();

	Disk(const Vec3 & p, const Vec3 & n, const real & r);

	Intersection
	intersect(const Ray & ray,
		  const real & t_min, const real & t_max) const noexcept;

	Vec3
	normal(const Intersection &) const noexcept;
    };
}
