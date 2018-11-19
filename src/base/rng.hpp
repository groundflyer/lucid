// -*- C++ -*-
// rng.hpp --
// Random number generation

#pragma once

#include <array>
#include <random>
#include <limits>
#include <algorithm>
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
		RandomDistribution(const T a, const T b) : m_dist(std::min(a, b), std::max(a, b)) {}

        template <typename Generator>
		auto
		operator()(Generator& g) noexcept
		{ return m_dist(g); }

		template <size_t N,
                  typename Generator,
				  template <typename, size_t> typename Container = std::array>
		auto
		operator()(Generator& g) noexcept
		{
			Container<T, N> ret;

			for (auto & v : ret)
				v = m_dist(g);

			return ret;
		}
    };

    template <>
    class RandomDistribution<bool>
    {
        std::bernoulli_distribution m_dist;

    public:
        RandomDistribution(double p) : m_dist(p) {}

        template <typename Generator>
        auto
        operator()(Generator& g) noexcept
        { return m_dist(g); }

		template <size_t N,
                  typename Generator,
				  template <typename, size_t> typename Container = std::array>
		auto
		operator()(Generator& g) noexcept
        {
            Container<bool, N> ret;

            for(auto& v : ret)
                v = m_dist(g);

            return ret;
        }
    };

    template <typename RealType,
              typename Generator,
              size_t bits = 8>
    auto
    rand(Generator& g)
    { return std::generate_canonical<RealType, bits>(g); }

    template <typename RealType,
              size_t N,
              typename Generator,
              size_t bits = 8,
			  template <typename, size_t> typename Container = std::array>
    auto
    rand(Generator& g)
    {
    	Container<RealType, N> ret;

    	for (auto &v : ret)
    	    v = std::generate_canonical<RealType, bits>(g);

    	return ret;
    }
}
