// -*- C++ -*-
// scene.hpp --
//

#pragma once

#include <structures/triangle_mesh.hpp>
#include "geometry_object.hpp"
#include "material.hpp"
#include "omni_light.hpp"

#include <vector>


namespace yapt
{
    class Scene
    {
	std::vector<TriangleMesh> meshes;
	std::vector<GeometryObject> objects;
	std::vector<Material> materials;
	std::vector<Light> lights;
    public:
	Scene();

	Scene(const size_t & nmeshes,
	      const size_t & nobjects,
	      const size_t & nmaterials,
	      const size_t & nlighs);

	const TriangleMesh &
	add_mesh(TriangleMesh && mesh) noexcept;

	const GeometryObject &
	add_object(GeometryObject && object) noexcept;

	const Material &
	add_material(Material && mat) noexcept;

	const Light &
	add_light(Light && light) noexcept;

	const std::vector<TriangleMesh> &
	get_meshes() const noexcept;

	const std::vector<GeometryObject> &
	get_objects() const noexcept;

	const std::vector<Material> &
	get_materials() const noexcept;

	const std::vector<Light> &
	get_lights() const noexcept;
    };
}
