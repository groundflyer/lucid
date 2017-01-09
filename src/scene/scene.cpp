// -*- C++ -*-
// scene.cpp --
//

#include "scene.hpp"


namespace yapt
{
    Scene::Scene() { }

    Scene::Scene(const size_t & nmeshes,
		 const size_t & nobjects,
		 const size_t & nmaterials,
		 const size_t & nlighs)
    {
	meshes.reserve(nmeshes);
	objects.reserve(nobjects);
	materials.reserve(nmaterials);
	lights.reserve(nlighs);
    }

    const TriangleMesh &
    Scene::add_mesh(TriangleMesh && mesh) noexcept
    {
	meshes.push_back(std::forward<TriangleMesh>(mesh));
	return meshes.back();
    }

    const GeometryObject &
    Scene::add_object(GeometryObject && object) noexcept
    {
	objects.push_back(std::forward<GeometryObject>(object));
	return objects.back();
    }

    const Material &
    Scene::add_material(Material && mat) noexcept
    {
	materials.push_back(std::forward<Material>(mat));
	return materials.back();
    }

    const Light &
    Scene::add_light(Light && light) noexcept
    {
	lights.push_back(std::forward<Light>(light));
	return lights.back();
    }

    const std::vector<TriangleMesh> &
    Scene::get_meshes() const noexcept
    {
	return meshes;
    }

    const std::vector<GeometryObject> &
    Scene::get_objects() const noexcept
    {
	return objects;
    }

    const std::vector<Material> &
    Scene::get_materials() const noexcept
    {
	return materials;
    }

    const std::vector<Light> &
    Scene::get_lights() const noexcept
    {
	return lights;
    }
}
