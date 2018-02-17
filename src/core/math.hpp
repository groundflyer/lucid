// -*- C++ -*-
// math.hpp
//

#pragma once

#include <cmath>
#include <iterator>
#include <type_traits>


namespace yapt::math
{
	using std::abs;
	using std::pow;
	using std::sqrt;
	using std::cos;
	using std::sin;

    template <typename T>
    const constexpr T PI = static_cast<T>(3.141592653589793);

    // simple power function
    template <typename T, unsigned exp>
    constexpr T
    pow(const T& base) noexcept
    {
		T ret = base;

		if (exp == 0)
			ret = 1;
		else if (exp > 1)
			for (unsigned _ = 1; _ < exp; ++_)
				ret *= base;

		return ret;
    }

    // check value is even
	template <typename T>
    constexpr typename std::enable_if_t<std::is_integral_v<T>, bool>
    is_even(const T& val) noexcept
    { return val % 2 == 0; }

    // return result of (-1)^a
    template <typename T, typename RT = int>
    constexpr RT
    minus_one_pow(const T & a)
    { return is_even(a) ? 1 : -1; }

    // return a sign of value
    template <typename T>
    constexpr T
    sign(const T & val) noexcept
    { return val > 0 ? 1 : -1; }


    // compute factorial of value
    constexpr size_t
    fac(const size_t & val) noexcept
    {
		std::size_t ret (1);

		if (val != 0)
			for (std::size_t i = 1; i <= val; ++i)
				ret *= i;

		return ret;
    }

    // compute inversion number, O(N^2)-version
    template <typename Iterable>
    constexpr unsigned
    inversion_number(const Iterable & input) noexcept
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
    template <typename Iterable, typename RT = int>
    constexpr RT
    sgn(const Iterable & numbers) noexcept
    {
    	RT ret {};

    	if (!has_equal(numbers))
			ret = minus_one_pow(inversion_number(numbers));

    	return ret;
    }

    template <typename T>
    constexpr T
    degrees(const T & _radians)
    { return _radians * static_cast<T>(180) / PI<T>; }

    template <typename T>
    constexpr T
    radians(const T & _degrees)
    { return _degrees * PI<T> / static_cast<T>(180); }
}
