// -*- C++ -*-
// primitive.hpp --
//

#pragma once

#include "ray.hpp"
#include "intersection.hpp"


namespace yapt
{
    class Intersection;


    class Primitive
    {
    public:
	Primitive() {}
	
	virtual	Intersection
	intersect(const Ray &,
		  const real &, const real &) const noexcept = 0;

	virtual Vec3
	normal(const Intersection &) const noexcept = 0;

	virtual Vec3
	tangent(const Intersection &) const noexcept = 0;
    };

}
