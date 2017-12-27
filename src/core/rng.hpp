// -*- C++ -*-
// rng.hpp --
// Random number generation

#pragma once

#include <random>
#include <limits>
#include <type_traits>

#ifndef RANDOM_BITS
	#define RANDOM_BITS 8
#endif


namespace yapt
{
    static thread_local std::random_device rd;
    static thread_local std::default_random_engine default_gen(rd());

    template <typename T = real, long long a = -1, long long b = 1>
    static thread_local std::uniform_real_distribution<T> rdist(static_cast<T>(a), static_cast<T>(b));

    template <typename T = int, T a = std::numeric_limits<T>::lowest(), T b = std::numeric_limits<T>::max()>
    static thread_local std::uniform_int_distribution<T> idist(a, b);


    template <typename T,
	      T a = std::numeric_limits<T>::lowest(), T b = std::numeric_limits<T>::max(),
	      typename Generator = std::default_random_engine,
	      typename Distribution = std::uniform_int_distribution<T>>
    typename std::enable_if_t<std::is_integral_v<T>, T>
    rand(Generator& g = default_gen, Distribution& dist = idist<T, a, b>)
    {
	return dist(g);
    }

    template <typename T,
	      long long a = 0, long long b = 1,
	      typename Generator = std::default_random_engine,
	      typename Distribution = std::uniform_real_distribution<T>>
    typename std::enable_if_t<std::is_floating_point_v<T>, T>
    rand(Generator& g = default_gen, Distribution& dist = rdist<T, a, b>)
    {
	if constexpr (a == 0 && b == 1)
	    return std::generate_canonical<T, RANDOM_BITS>(g);
	else
	    return dist(g);
    }
}
