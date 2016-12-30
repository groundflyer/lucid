// -*- C++ -*-
// triangle_mesh.h


#pragma once

#include <core/basic_types.hpp>
#include <vector>


namespace yapt
{
    using TriangleID = std::array<size_t, 3>;

    // interpolated normal
    // constexpr Vec3
    // phong_normal(const Vec3 & n0, const Vec3 & n1, const Vec3 & n2,
    // 		 const Vec3 & uvw)
    // { return (n0 * uvw.x() + n1 * uvw.y() + n2 * uvw.z()); }


    class TriangleMesh
    {
	bool _have_normals = false;
	bool _have_uv = false;

	std::vector<Vec3> vertices_p; // vertices positions
	std::vector<Vec3> vertices_n; // vertices normals
	std::vector<Vec3> vertices_uvw; // texture coordinates
	std::vector<TriangleID> triangles;

    public:
	TriangleMesh();

	TriangleMesh(const size_t vertex_num,
		     const size_t tri_num,
		     const bool store_normals = false,
		     const bool store_uv = false);

	void
	add_vertex(const Vec3 & p) noexcept;

	void
	add_vertex(const Vec3 & p, const Vec3 & n) noexcept;

	void
	add_vertex(const Vec3 & p, const Vec3 & n, const Vec3 & uvw) noexcept;

	void
	add_triangle(const size_t v0, const size_t v1, const size_t v2) noexcept;

	void
	add_triangle(const TriangleID & id) noexcept;

	const Vec3&
	get_vertex_pos(const size_t tri_id, const size_t vtx) const noexcept;

	Vec3
	get_triangle_normal(const size_t tri_id) const noexcept;
    };
}
