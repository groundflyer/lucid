// -*- C++ -*-
// rng.hpp --
// Random number generation

#pragma once

#include <array>
#include <random>
#include <limits>
#include <type_traits>


namespace yapt
{
    template <typename T,
			  typename Distribution = std::conditional_t<std::is_integral_v<T>,
														 std::uniform_int_distribution<T>,
														 std::uniform_real_distribution<T>>>
    class RandomDistribution
    {
		Distribution m_dist;

    public:
		RandomDistribution(const T a, const T b) : m_dist(a, b) {}

        template <typename Generator>
		auto
		operator()(Generator&& gen) noexcept
		{ return m_dist(std::forward<Generator>(gen)); }

		template <size_t N,
                  typename Generator,
				  template <typename, size_t> typename Container = std::array>
		auto
		operator()(Generator&& gen) noexcept
		{
			Container<T, N> ret;

			for (auto & v : ret)
				v = m_dist(std::forward<Generator>(gen));

			return ret;
		}
    };

    // template <typename T>
    // RandomDistribution(const T, const T) -> RandomDistribution<T>;

    template <typename RealType,
              typename Generator,
              size_t bits = 8>
    auto
    rand(Generator&& gen)
    { return std::generate_canonical<RealType, bits>(gen); }

    template <typename RealType,
              size_t N,
              typename Generator,
              size_t bits = 8,
			  template <typename, size_t> typename Container = std::array>
    auto
    rand(Generator&& gen)
    {
    	Container<RealType, N> ret;

    	for (auto &v : ret)
    	    v = std::generate_canonical<RealType, bits>(std::forward<Generator>(gen));

    	return ret;
    }
}
