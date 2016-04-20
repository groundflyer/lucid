// -*- C++ -*-
// triangle_mesh.cpp


#include "triangle_mesh.h"


namespace yapt
{
    TriangleMesh::TriangleMesh() {}

    TriangleMesh::TriangleMesh(const size_t vertex_num,
			       const size_t tri_num,
			       const bool store_normals,
			       const bool store_uv) :
	_have_normals(store_normals), _have_uv(store_uv)
    {
	vertices_p.reserve(vertex_num);
	triangles.reserve(tri_num);

	if (store_normals)
	    vertices_n.reserve(vertex_num);

	if (store_uv)
	    vertices_uvw.reserve(vertex_num);
    }

    void
    TriangleMesh::add_vertex(const Vec3 & p) noexcept
    {
	vertices_p.push_back(p);
    }

    void
    TriangleMesh::add_vertex(const Vec3 & p, const Vec3 & n) noexcept
    {
	add_vertex(p);
	vertices_n.push_back(n);
    }

    void
    TriangleMesh::add_vertex(const Vec3 & p, const Vec3 & n, const Vec3 & uvw) noexcept
    {
	add_vertex(p, n);
	vertices_uvw.push_back(uvw);
    }

    void
    TriangleMesh::add_triangle(const size_t v0, const size_t v1, const size_t v2) noexcept
    { triangles.push_back(TriangleID({v0, v1, v2})); }

    void
    TriangleMesh::add_triangle(const TriangleID & id) noexcept
    { triangles.push_back(id); }

    // return position of vertex vtx of triangle tri_id
    const Vec3&
    TriangleMesh::get_vertex_pos(const size_t tri_id,
				 const size_t vtx) const noexcept
    {
	ASSERT(vtx <= 2);

	return vertices_p[triangles[tri_id][vtx]];
    }

    Vec3
    TriangleMesh::get_triangle_normal(const size_t tri_id) const noexcept
    {
	return normalize(vertices_p[triangles[tri_id][0]]
			 - vertices_p[triangles[tri_id][1]])
	    ^ normalize(vertices_p[triangles[tri_id][0]]
			- vertices_p[triangles[tri_id][2]]);
    }

    // Intersection
    // TriangleMesh::intersect(const Ray & ray,
    // 			    const real t_min, const real t_max) const noexcept
    // {
    // 	bool intersected = false;
    // 	real t = 0;
    // 	Vec2 uv;
    // 	Triangle hit {};

    // 	// choose the closest intersected triangle
    // 	for (auto tri : triangles)
    // 	    {
    // 		real next_t = 0;
    // 		Vec2 next_uv;

    // 		intersected = intersect_triangle(ray, t_min, t_max,
    // 						 vertices_p[tri[0]],
    // 						 vertices_p[tri[1]],
    // 						 vertices_p[tri[2]],
    // 						 next_t, uv);
    // 		if (intersected && (next_t < t))
    // 		    {
    // 			t = next_t;
    // 			uv = next_uv;
    // 			hit = tri;
    // 		    }
    // 	    }

    // 	Vec3 normal;

    // 	if (intersected)
    // 	    {
    // 		if (_have_normals)
    // 		    // get smooth normal from normal info
    // 		    normal = phong_normal(vertices_n[hit[0]],
    // 					  vertices_n[hit[1]],
    // 					  vertices_n[hit[2]],
    // 					  Vec3(uv.u(), uv.v(), 1-uv.u()-uv.v()));
    // 		else
    // 		    // compute normal using cross product of edges
    // 		    normal =
    // 			normalize(vertices_p[hit[0]] - vertices_p[hit[1]]) ^
    // 			normalize(vertices_p[hit[0]] - vertices_p[hit[2]]);
    // 	    }

    // 	return Intersection(intersected, t, normal, uv);
    // }
}
