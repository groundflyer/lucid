// -*- C++ -*-
// misc.hpp --
//

#pragma once

#include <limits>
#include <type_traits>


namespace yapt
{
    template <typename T>
    class Range
    {
		static_assert(std::is_arithmetic<T>::value, "T is not arithmetic!");

		const T m_begin = std::numeric_limits<T>::min();
		const T m_end = std::numeric_limits<T>::max();

    public:
		constexpr
		Range() {};

		constexpr
		Range(const T& begin, const T& end)
		: m_begin(std::min(begin, end)), m_end(std::max(end, begin)) {}

		constexpr bool
		encloses(const T & val) const noexcept
		{ return (val >= m_begin) && (val <= m_end); }
    };
}
