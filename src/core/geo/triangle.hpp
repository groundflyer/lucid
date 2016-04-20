// -*- C++ -*-
// triangle.h --
//

#pragma once

#include <structures/triangle_mesh.h>


namespace yapt
{
    class Triangle : public Primitive
    {
	TriangleMesh * _mesh = nullptr;
	size_t _id;

    public:
	Triangle() {}

	Triangle(TriangleMesh * mesh, const size_t id)
	    : _mesh(mesh), _id(id) {}

	Intersection
	intersect(const Ray & ray,
		  const real t_min, const real t_max) const noexcept
	{
	    ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);

	    const Vec3 & p0 = _mesh->get_vertex_pos(_id, 0);
	    const Vec3 & p1 = _mesh->get_vertex_pos(_id, 1);
	    const Vec3 & p2 = _mesh->get_vertex_pos(_id, 2);

	    bool intersected = false;

	    const Vec3 edge1 = p1 - p0;
	    const Vec3 edge2 = p2 - p0;
	    const Vec3 pvec = ray.dir() ^ edge2;
	    const real D = edge1 % pvec;
	    real t(0);
	    real u(0);
	    real v(0);

	    if (D > 0)
		{
		    const Vec3 tvec = ray.origin() - p0;
		    u = tvec % pvec;

		    if (in_range(u, real(0), D))
			{
			    const Vec3 qvec = tvec ^ edge1;
			    v = ray.dir() % qvec;

			    if (v >= 0 && u + v <= D)
				{
				    t = edge2 % qvec;
				    intersected = in_range(t, t_min, t_max);

				    if (intersected)
					{
					    const real inv_D = real(1) / D;
					    t *= inv_D;
					    u *= inv_D;
					    v *= inv_D;
					}
				}
			}
			
		}

	    return Intersection(intersected, t, Vec2(u, v));
	}
    };


    // basic ray-triangle intersection
    // constexpr bool
    // intersect_triangle(const Ray & ray, const real t_min, const real t_max,
    // 		       const Vec3 & p0, const Vec3 & p1, const Vec3 & p2,
    // 		       real & t, Vec2 & uv) noexcept
    // {
    // 	bool intersected = false;

    // 	const Vec3 edge1 = p1 - p0;
    // 	const Vec3 edge2 = p2 - p0;
    // 	const Vec3 pvec = ray.dir() ^ edge2;
    // 	const real D = edge1 % pvec;
    // 	real u(0);
    // 	real v(0);

    // 	if (D > 0)
    // 	    {
    // 		const Vec3 tvec = ray.origin() - p0;
    // 		u = tvec % pvec;

    // 		if (in_range(u, real(0), D))
    // 		    {
    // 			const Vec3 qvec = tvec ^ edge1;
    // 			v = ray.dir() % qvec;

    // 			if (v >= 0 && u + v <= D)
    // 			    {
    // 				t = edge2 % qvec;
    // 				intersected = in_range(t, t_min, t_max);

    // 				if (intersected)
    // 				    {
    // 					const real inv_D = real(1) / D;
    // 					t *= inv_D;
    // 					u *= inv_D;
    // 					v *= inv_D;
    // 				    }
    // 			    }
    // 		    }
			
    // 	    }

    // 	uv = Vec2(u, v);
    // 	return intersected;
    // }

    // interpolated normal
    constexpr Vec3
    phong_normal(const Vec3 & n0, const Vec3 & n1, const Vec3 & n2,
		 const Vec3 & uvw)
    { return (n0 * uvw.x() + n1 * uvw.y() + n2 * uvw.z()); }
}
