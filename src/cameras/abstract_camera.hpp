// -*- C++ -*-
// abstract_camera.hpp --
//

#ifndef __ABSTRACT_CAMERA__
#define __ABSTRACT_CAMERA__


#include <core/geo/ray.hpp>


namespace yapt
{
    class AbstractCamera
    {
    public:
	AbstractCamera() {};

	virtual Ray
	generate_ray(const Vec2 &) const noexcept = 0;
    };
}

#endif // __ABSTRACT_CAMERA__
