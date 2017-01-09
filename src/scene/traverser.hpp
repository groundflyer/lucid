// -*- C++ -*-
// intersector.hpp --
//

#pragma once

#include "scene.hpp"


namespace yapt
{
    class Traverser
    {
	const Scene * _scene = nullptr;
	const real _t_min = 0;
	const real _t_max = std::numeric_limits<real>::infinity();
    public:
	Traverser();

	explicit
	Traverser(const Scene & scene);

	Traverser(const Scene & scene, const real & t_min, const real & t_max);

	std::pair<Intersection, const GeometryObject&>
	traverse(const Ray & ray) const noexcept;
    };
}
