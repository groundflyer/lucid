// -*- C++ -*-
// material.cpp --


#include <tuple>		// std::tie
#include "scene.hpp"
#include "material.hpp"


namespace yapt
{
    Material::Material(const RGB& clr): _diff(clr) { }

    Material::Material() { }

    RGB
    Material::eval(const Intersection & isect, const Scene & scene) const noexcept
    {
	RGB accum (0);
	for (const Light & light : scene.get_lights()) {
	    RGB lcolor;
	    Vec3 lpos;
	    std::tie(lcolor, lpos) = light.sample(isect.pos());
	    const Vec3 dir = normalize(lpos - isect.pos());
	    accum += lcolor * isect.prim().normal(isect) % dir;
	}
	return _diff * accum;
    }
}
