// -*- C++ -*-
// math.hpp
//

#pragma once

#include <cmath>
#include <limits>
#include <iterator>
#include <type_traits>


namespace yapt::math
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
    const constexpr T PI = T{3.141592653589793};

    // simple power function
    template <unsigned exp, typename T>
    constexpr T
    pow(const T base) noexcept
    {
		T ret = base;

		if (exp == 0)
			ret = 1;
		else if (exp > 1)
			for (unsigned _ = 1; _ < exp; ++_)
				ret *= base;

		return ret;
    }

    template <unsigned exp, typename T>
    constexpr T
    ct_pow(const T value) noexcept
    {
        if(!exp) {
            return 1;
        } else {
            const T base = ct_pow<exp/2>(value);
            if constexpr (exp & 1)
                return base * base * value;
            else
                return base * base;
        }
    }

    // check value is even
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
    constexpr size_t
    fac(const size_t val) noexcept
    {
		size_t ret{1};

		if (val != 0)
			for (size_t i = 1; i <= val; ++i)
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

    template <typename T>
    constexpr T
    degrees(const T _radians)
    { return _radians * static_cast<T>(180) / PI<T>; }

    template <typename T>
    constexpr T
    radians(const T _degrees)
    { return _degrees * PI<T> / static_cast<T>(180); }

    template <typename T, typename Bias>
    constexpr auto
    lerp(const T a, const T b, const Bias bias)
    { return b * bias + a * (T(1) - bias); }

    template <typename T, typename ULP = unsigned>
    constexpr typename std::enable_if_t<std::is_floating_point_v<T>, bool>
    almost_equal(const T a, const T b, const ULP ulp = 2)
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
    fit(const T val, const T bottom = T{0}, const T top = T{1})
    {
        const auto range = (top - bottom);
        return (val - bottom) / range;
    }
}
