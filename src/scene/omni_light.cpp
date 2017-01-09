// -*- C++ -*-
// omni_light.hpp --

#include "omni_light.hpp"


namespace yapt
{
    Light::Light() {}

    Light::Light(const Vec3 & pos, const RGB & clr): _pos(pos), _clr(clr) {}

    std::pair<RGB, Vec3>
    Light::sample(const Vec3 & pos) const noexcept
    {
	return std::pair<RGB, Vec3>(_clr * (static_cast<real>(1) + pow<real, 2>(distance(pos, _pos))), _pos);
    }
}
