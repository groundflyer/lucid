// -*- C++ -*-
// material.hpp --
//

#pragma once

#include <core/basic_types.hpp>


namespace yapt
{
    class Material
    {
	RGB _diff;

    public:
	Material() {};

	void
	set_diff(const RGB & clr) : _diff(clr) {};
    };

}
