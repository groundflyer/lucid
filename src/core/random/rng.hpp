// -*- C++ -*-
// rng.hpp --
// Random number generation

#pragma once

#include <random>
#include <tuple>
#include <core/basic_types.hpp>


#ifndef RANDOM_BITS
	#define RANDOM_BITS 10
#endif


namespace yapt
{
    static thread_local std::random_device rd;
    static thread_local std::default_random_engine gen(rd());
    static thread_local std::uniform_real_distribution<real> rand11(-1, 1);

    real rand();

    // zero-centered vector within range (-1, 1)
    Vec3 rand_vec3();

    Vec2 rand_vec2();

    RGB rand_rgb();
}
