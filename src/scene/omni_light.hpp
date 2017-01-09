// -*- C++ -*-
// omni_light.hpp --
//

#pragma once

#include <core/basic_types.hpp>


namespace yapt
{
    class Light
    {
	Vec3 _pos;
	RGB _clr;
    public:
	Light();

	Light(const Vec3 & pos, const RGB & clr);

 	std::pair<RGB, Vec3>
	sample(const Vec3 & pos) const noexcept;
    };
}
