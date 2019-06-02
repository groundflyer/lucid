// -*- C++ -*-
// math.hpp
//

#pragma once

#include <cmath>
#include <limits>
#include <utility>
#include <iterator>
#include <algorithm>
#include <type_traits>


namespace lucid
{
namespace math
{
	using std::abs;
	using std::pow;
	using std::sqrt;
	using std::cos;
	using std::sin;
    using std::tan;
    using std::fmod;
    using std::floor;
    using std::ceil;
    using std::round;

    template <typename T>
    const constexpr T Pi_ = T{3.141592653589793};
}

	template <typename T>
    constexpr typename std::enable_if_t<std::is_integral_v<T>, bool>
    is_even(const T val) noexcept
    { return val % 2 == 0; }

    // return result of (-1)^a
    template <typename T>
    constexpr int
    minus_one_pow(const T a)
    { return is_even(a) ? 1 : -1; }

    // factorial
    constexpr std::size_t
    fac(const std::size_t val) noexcept
    {
		std::size_t ret{1};

		if (val != 0)
			for (std::size_t i = 1; i <= val; ++i)
				ret *= i;

		return ret;
    }

    // compute inversion number, O(N^2)-version
    template <typename Iterable>
    constexpr unsigned
    inversion_number(const Iterable& input) noexcept
    {
		unsigned ret = 0;

		for (auto i = input.cbegin(); i != input.cend() - 1; ++i)
			for (auto j = std::next(i); j != input.cend(); ++j)
				if (*i > *j)
					++ret;

		return ret;
    }

    // const iterator version
    template <typename Iterable>
    constexpr bool
    has_equal(const Iterable& input) noexcept
    {
		for (auto i = input.cbegin(); i != input.cend() - 1; ++i)
			for (auto j = std::next(i); j != input.cend(); ++j)
				if (*i == *j)
					return true;
		return false;
	}


    // compute Levi-Civita symbol (signature of permutation)
    template <typename Iterable>
    constexpr int
    sgn(const Iterable& numbers) noexcept
    {
    	int ret{};

    	if (!has_equal(numbers))
			ret = minus_one_pow(inversion_number(numbers));

    	return ret;
    }

    template <typename T, typename Bias>
    constexpr auto
    lerp(const T a, const T b, const Bias bias)
    { return b * bias + a * (T{1} - bias); }

    template <typename T, typename ULP>
    constexpr typename std::enable_if_t<std::is_floating_point_v<T>, bool>
    almost_equal(const T a, const T b, const ULP ulp)
    {
        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        // unless the result is subnormal
        const auto amb = abs(a - b);
        return amb <= std::numeric_limits<T>::epsilon() * abs(a + b) * ulp ||
            amb < std::numeric_limits<T>::min();
    }

    template <typename T>
    constexpr auto
    fit(const T val, const T minval, const T maxval)
    { return (val - minval) / (maxval - minval); }

    template <unsigned exp, typename T>
    constexpr T
    pow(const T value) noexcept
    {
        if(!exp) {
            return 1;
        } else {
            const T base = pow<exp/2>(value);
            if constexpr (is_even(exp))
                return base * base;
            else
                return base * base * value;
        }
    }

    template <typename T>
    constexpr T
    degrees(const T _radians)
    { return _radians * T{180} / math::Pi_<T>; }

    template <typename T>
    constexpr T
    radians(const T _degrees)
    { return _degrees * math::Pi_<T> / T{180}; }

    // solves quadratic equation ax^2 + bx + c
    // for non-complex cases
    // return bool and the smallest of two positive roots
    // template <typename T>
    // constexpr auto
    // quadratic(const T a, const T b, const T c) noexcept
    // {
    //     const auto D = pow<2>(b) - T{4} * a * c;
    //     const auto sqrtD = std::copysign(math::sqrt(D), b);
    //     const auto q = -T{0.5} * a * (b + sqrtD);
    //     const auto [x1, x2] = std::minmax(c / q, q / a);
    //     return std::pair{!std::signbit(D), std::signbit(x1) ? x1 : x2};
    // };

    template <typename T>
    constexpr auto
    quadratic(const T a, const T b, const T c) noexcept
    {
        const auto D = pow<2>(b) - T{4} * a * c;
        const auto factor = T{0.5} * a;
        const auto sqrtD = math::sqrt(D);
        const auto x1 = (-b + sqrtD) * factor;
        const auto x2 = (-b - sqrtD) * factor;
        const auto x = std::signbit(x1) ? x2 : (std::signbit(x2) ? x1 : std::min(x1, x2));
        return std::pair{!std::signbit(D * x), x};
    }
}
