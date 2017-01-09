// -*- C++ -*-
// material.hpp --
//

#pragma once

#include <core/basic_types.hpp>
#include <core/geo/intersection.hpp>
#include "omni_light.hpp"


namespace yapt
{
    // forward declaration
    class Scene;

    class Material
    {
	RGB _diff;

    public:
	Material();

	explicit
	Material(const RGB & clr);

	RGB
	eval(const Intersection & isect, const Scene & scene) const noexcept;
    };
}
