// -*- C++ -*-
// traverser.cpp -- 

#include "traverser.hpp"


namespace yapt
{
    Traverser::Traverser() { }

    Traverser::Traverser(const Scene & scene) : _scene(&scene) { }

    Traverser::Traverser(const Scene & scene, const real & t_min, const real & t_max) : _scene(&scene), _t_min(t_min), _t_max(t_max) { }

    std::pair<Intersection, const GeometryObject&>
    Traverser::traverse(const Ray & ray) const noexcept
    {
	ASSERT(scene);

	GeometryObject const * closest_object = nullptr;
	Intersection isect {false};
	for (const GeometryObject & object : _scene->get_objects()) {
	    for (const auto & prim : object.get_primitives()) {
		Intersection n_isect = prim->intersect(object.get_transform().inverse()(ray), _t_min, _t_max);
		if (n_isect && (n_isect.t() < isect.t())) {
		    isect = n_isect;
		    closest_object = &object;
		}
	    }
	}

	return std::pair<Intersection, const GeometryObject&>(isect, *closest_object);
    }
}
