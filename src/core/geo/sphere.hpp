// -*- C++ -*-
// sphere.hpp --
//

#pragma once

#include "primitive.hpp"


namespace yapt
{
    class Sphere : Primitive
    {
	Vec3 _p; 		// position
	real _r = 1;		// radius
    public:
	Sphere();

	Sphere(const Vec3 & p, const real r);

	Intersection
	intersect(const Ray & ray,
		  const real & t_min, const real & t_max) const noexcept;

	Vec3
	normal(const Intersection & isect) const noexcept;

	Vec3
	tangent(const Intersection & isect) const noexcept;
    };
}
