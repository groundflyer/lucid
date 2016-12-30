// -*- C++ -*-
// bbox.cpp --
//

#include "bbox.hpp"


namespace yapt
{
    AABB::AABB() {}

    AABB::AABB(const Vec3 & vmin, const Vec3 & vmax) : _vmin(vmin),_vmax(vmax) {}

    Intersection
    AABB::intersect(const Ray & ray,
		    const real & t_min, const real & t_max) const noexcept
    {
	ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);

	real xmin = 0;
	real xmax = 0;
	real ymin = 0;
	real ymax = 0;
	real zmin = 0;
	real zmax = 0;

	const auto invdir = Vec3(1) / ray.dir();

	const short signx = invdir.x() < 0;
	const short signy = invdir.y() < 0;
	const short signz = invdir.z() < 0;

	const Vec3 bounds[2] {_vmin, _vmax};

	xmin = (bounds[signx].x() - ray.origin().x()) * invdir.x();
	xmax = (bounds[1-signx].x() - ray.origin().x()) * invdir.x();

	ymin = (bounds[signy].y() - ray.origin().y()) * invdir.y();
	ymax = (bounds[1-signy].y() - ray.origin().y()) * invdir.y();

	if ((xmin > ymax) || (ymin > xmax))
	    return Intersection(false);

	if (ymin > xmin) xmin = ymin;
	if (ymax < xmax) xmax = ymax;

	zmin = (bounds[signz].z() - ray.origin().z()) * invdir.z();
	zmax = (bounds[1-signz].z() - ray.origin().z()) * invdir.z();

	if ((xmin > zmax) || (zmin > xmax))
	    return Intersection(false);

	if (zmin > xmin) xmin = zmin;
	if (zmax < xmax) xmax = zmax;

	bool intersected = in_range(xmin, t_min, t_max);
	return Intersection(intersected, xmin, ray, this);
    }

    Vec3
    AABB::normal(const Intersection & isect) const noexcept
    {
	// TODO find normal
	// it's just a mock
	auto tmp = isect.pos() - _vmin;
	return normalize(tmp);
    }
}
