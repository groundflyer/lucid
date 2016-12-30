// -*- C++ -*-
// material.hpp --
//

#pragma once

#include <core/basic_types.hpp>
#include <core/geo/intersection.hpp>


namespace yapt
{
    class Material
    {
	RGB _diff;

    public:
	Material();

	explicit
	Material(const RGB & clr);

	RGB
	eval(const Intersection & isect) const noexcept;
    };

}
