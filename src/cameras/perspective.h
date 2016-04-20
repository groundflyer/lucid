// -*- C++ -*-
// perspective.h
//

#ifndef __PERSPECTIVE_CAMERA__
#define __PERSPECTIVE_CAMERA__


#include "abstract_camera.hpp"
#include <core/math/transforms.hpp>


namespace yapt
{
    class PerspectiveCamera : public AbstractCamera
    {
	Vec3 p = Vec3(0);	// position
	Vec3 dir = Vec3(0,0,1); // direction
	real tan_fovx_half = 1;
	real tan_fovy_half = 1;

    public:
	PerspectiveCamera();

	// the angles are in radians
	PerspectiveCamera(const real & fovx,
			  const real & fovy);

	PerspectiveCamera(const Transform & transform,
			  const real & fovx,
			  const real & fovy);

	void
	set_transform(const Transform & transform) noexcept;

	void
	push_transform(const Transform & transform) noexcept;

	Ray
	generate_ray(const Vec2 & ndc) const noexcept;
    };
}


#endif // __PERSPECTIVE_CAMERA__
