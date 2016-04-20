// -*- C++ -*-
// primitive.hpp --
//

#pragma once

#include "ray.hpp"
#include "intersection.hpp"


namespace yapt
{
    class Primitive
    {
    public:
	constexpr
	Primitive() {}
	
	virtual constexpr
	Intersection
	intersect(const Ray & ray,
		  const real t_min, const real t_max) const noexcept = 0;
    };

}
