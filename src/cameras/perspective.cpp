// -*- C++ -*-
// perspective.cpp -- 

#include "perspective.h"
// #include <cmath>


namespace yapt
{
    PerspectiveCamera::PerspectiveCamera() {}

    PerspectiveCamera::PerspectiveCamera(const real & fovx,
					 const real & fovy)
    {
	tan_fovx_half = std::tan(fovx * 0.5);
	tan_fovy_half = std::tan(fovy * 0.5);
    }

    PerspectiveCamera::PerspectiveCamera(const Transform & transform,
					 const real & fovx,
					 const real & fovy)
    {
	tan_fovx_half = std::tan(fovx * 0.5);
	tan_fovy_half = std::tan(fovy * 0.5);
	p = transform.as_point(Vec3(0));
	dir = transform.as_normal(Vec3(0,0,1));
    }

    void
    PerspectiveCamera::set_transform(const Transform & transform) noexcept
    {
	p = transform.as_point(Vec3(0));
	dir = transform.as_normal(Vec3(0,0,1));
    }

    void
    PerspectiveCamera::push_transform(const Transform & transform) noexcept
    {
	p = transform.as_point(p);
	dir = transform.as_normal(dir);
    }

    Ray
    PerspectiveCamera::generate_ray(const Vec2 & ndc) const noexcept
    {
	ASSERT(((ndc.u() >= 0) && (ndc.u() <= 1)) ||
	       ((ndc.v() >= 0) && (ndc.v() <= 1)));

	Vec3 p = Vec3((ndc.u() - 0.5) * tan_fovx_half,
		      (0.5 - ndc.v()) * tan_fovy_half,
		      1);

	return Ray(Vec3(0), normalize(p));
    }
}
