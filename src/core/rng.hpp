// -*- C++ -*-
// rng.hpp --
// Random number generation

#pragma once

#include <array>
#include <random>
#include <limits>
#include <type_traits>

#ifndef RANDOM_BITS
	#define RANDOM_BITS 10
#endif


namespace yapt
{
    template <typename T,
			  typename RandomDevice = std::random_device,
			  typename Generator = std::default_random_engine,
			  typename Distribution = std::conditional_t<std::is_integral_v<T>,
														 std::uniform_int_distribution<T>,
														 std::uniform_real_distribution<T>>>
    class RNG
    {
		RandomDevice	m_rd;
		Generator		m_gen{m_rd()};
		Distribution	m_dist{std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max()};

    public:
		RNG(const T& a, const T& b) : m_dist(a, b) {}

		RNG(const RNG&) = delete;

		RNG&
		operator=(const RNG&) = delete;

		auto
		operator()() noexcept
		{ return m_dist(m_gen); }

		template <size_t N,
				  template <typename, size_t> typename Container = std::array>
		auto
		operator()() noexcept
		{
			Container<T, N> ret;

			for (auto & v : ret)
				v = m_dist(m_gen);

			return ret;
		}
    };


    // canonical real type random number generator
    template <typename RealType,
			  typename RandomDevice = std::random_device,
    	      typename Generator = std::default_random_engine>
    typename std::enable_if_t<std::is_floating_point_v<RealType>, RealType>
    rand()
    {
		static thread_local RandomDevice rd;
		static thread_local Generator gen(rd());

		return std::generate_canonical<RealType, RANDOM_BITS>(gen);
    }

    template <typename RealType, size_t N,
			  template <typename, size_t> typename Container = std::array,
    	      typename RandomDevice = std::random_device,
    	      typename Generator = std::default_random_engine>
    typename std::enable_if_t<std::is_floating_point_v<RealType>, Container<RealType, N>>
    rand()
    {
    	Container<RealType, N> ret;

    	for (auto & v : ret)
    	    v = rand<std::decay_t<decltype(v)>, RandomDevice, Generator>();

    	return ret;
    }
}
