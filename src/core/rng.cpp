// -*- C++ -*-
// rng.cpp --
// random number generation

#include "rng.hpp"


namespace yapt
{
    real rand()
    {
	return std::generate_canonical<real, RANDOM_BITS>(gen);
    }

    Vec3 rand_vec3()
    {
	return Vec3(rand11(gen), rand11(gen), rand11(gen));
    }

    Vec2 rand_vec2()
    {
	return Vec2(std::generate_canonical<real, RANDOM_BITS>(gen),
		    std::generate_canonical<real, RANDOM_BITS>(gen));
    }

    RGB rand_rgb()
    {
	return RGB(std::generate_canonical<float, RANDOM_BITS>(gen),
		   std::generate_canonical<float, RANDOM_BITS>(gen),
		   std::generate_canonical<float, RANDOM_BITS>(gen));
    }
}
