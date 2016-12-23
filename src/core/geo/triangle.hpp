// -*- C++ -*-
// triangle.h --
//

#pragma once

#include "primitive.hpp"
#include <structures/triangle_mesh.h>


namespace yapt
{
    class Triangle : public Primitive
    {
	TriangleMesh * _mesh = nullptr;
	size_t _id;

    public:
	Triangle();

	Triangle(TriangleMesh * mesh, const size_t id);

	Intersection
	intersect(const Ray & ray,
		  const real & t_min, const real & t_max) const noexcept;
    };


    // interpolated normal
    constexpr Vec3
    phong_normal(const Vec3 & n0, const Vec3 & n1, const Vec3 & n2,
		 const Vec3 & uvw)
    { return (n0 * uvw.x() + n1 * uvw.y() + n2 * uvw.z()); }
}
