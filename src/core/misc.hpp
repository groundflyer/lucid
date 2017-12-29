// -*- C++ -*-
// misc.hpp --
//

#pragma once

#include "pi.hpp"

#include <limits>
#include <algorithm>
#include <type_traits>


namespace yapt
{
    // simple power function
    template <typename T, unsigned exp>
    constexpr T
    pow(const T & base) noexcept
    {
		T ret = base;

		if (exp == 0)
			ret = 1;
		else if (exp > 1)
			for (unsigned _ = 1; _ < exp; ++_)
				ret *= base;

		return ret;
    }

    template <typename T>
    constexpr T
    pow(const T & base, const unsigned & exp) noexcept
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
    constexpr bool
    is_even(const int & val) noexcept
    { return val % 2 == 0; }


    // compute inversion number, O(N^2)-version
    template <typename RandomIt>
    constexpr unsigned
    inversion_number(const RandomIt first, const RandomIt last) noexcept
    {
		unsigned ret = 0;

		for (auto i = first; i != last - 1; ++i)
			for (auto j = std::next(i); j != last; ++j)
				if (*i > *j)
					++ret;

		return ret;
    }

    // const iterator version
    template <typename Iterable>
    constexpr unsigned
    inversion_number(const Iterable & input) noexcept
    { return inversion_number(input.cbegin(), input.cend()); }

    // check if there are equal elements in input, O(N^2)-version
    template <typename RandomIt>
    constexpr bool
    has_equal(const RandomIt first, const RandomIt last) noexcept
    {
		bool ret = false;

		for (auto i = first; i != last - 1; ++i)
			for (auto j = std::next(i); j != last; ++j)
				if (*i == *j)
					ret = true;

		return ret;
    }

    // const iterator version
    template <typename Iterable>
    constexpr bool
    has_equal(const Iterable & input) noexcept
    { return has_equal(input.cbegin(), input.cend()); }


    // return result of (-1)^a
    template <typename T, typename RT = int>
    constexpr RT
    minus_one_pow(const T & a)
    { return is_even(a) ? 1 : -1; }


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


    // return a sign of value
    template <typename T>
    constexpr T
    sign(const T & val) noexcept
    { return val > 0 ? 1 : -1; }


    // compute factorial of value
    constexpr size_t
    fac(const size_t & val) noexcept
    {
		size_t ret (1);

		if (val != 0)
			for (size_t i = 1; i <= val; ++i)
				ret *= i;

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


    template <typename T>
    class Range
    {
		static_assert(std::is_arithmetic<T>::value, "T is not arithmetic");

		const T m_begin;
		const T m_end;

    public:
		constexpr
		Range() = delete;

		constexpr
		Range(const T & begin = std::numeric_limits<T>::lowest(),
			  const T & end = std::numeric_limits<T>::max())
		: m_begin(std::min(begin, end)), m_end(std::max(end, begin)) {}

		constexpr bool
		encloses(const T & val) const noexcept
		{ return (val >= m_begin) && (val <= m_end); }
    };
}
