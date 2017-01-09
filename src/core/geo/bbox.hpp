// -*- C++ -*-
// bbox.hpp --
//

#pragma once

#include "primitive.hpp"


namespace yapt
{
    // axis-aligned bounding box
    class AABB : public Primitive
    {
	Vec3 _vmin;
	Vec3 _vmax;
    public:
	AABB();

	AABB(const Vec3 & vmin, const Vec3 & vmax);

	Intersection
	intersect(const Ray & ray,
		  const real & t_min, const real & t_max) const noexcept;

	Vec3
	normal(const Intersection &) const noexcept;

	Vec3
	tangent(const Intersection &) const noexcept;
    };

}
