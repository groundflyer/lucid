// -*- C++ -*-
// triangle.cpp -- 

#include "triangle.hpp"


namespace yapt
{
    Triangle::Triangle() {}

    Triangle::Triangle(TriangleMesh * mesh, const size_t id) : _mesh(mesh), _id(id) {}

    Intersection
    Triangle::intersect(const Ray & ray,
			const real & t_min, const real & t_max) const noexcept
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

		if (in_range(u, static_cast<real>(0), D))
		    {
			const Vec3 qvec = tvec ^ edge1;
			v = ray.dir() % qvec;

			if (v >= 0 && u + v <= D)
			    {
				t = edge2 % qvec;
				intersected = in_range(t, t_min, t_max);

				if (intersected)
				    {
					const real inv_D = static_cast<real>(1) / D;
					t *= inv_D;
					u *= inv_D;
					v *= inv_D;
				    }
			    }
		    }
			
	    }

	return Intersection(intersected, t, ray, this, Vec2(u, v));
    }

    Vec3
    Triangle::normal(const Intersection &) const noexcept
    {
	const Vec3 p0 = _mesh->get_vertex_pos(_id, 0);
	return normalize((p0 -_mesh->get_vertex_pos(_id, 1))
			 ^ (p0 - _mesh->get_vertex_pos(_id, 2)));
    }

    Vec3
    Triangle::tangent(const Intersection &) const noexcept
    {
	return normalize(_mesh->get_vertex_pos(_id, 1) - _mesh->get_vertex_pos(_id, 2));
    }
}
