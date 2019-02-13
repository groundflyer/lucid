// -*- C++ -*-
// arrayview.hpp --
//

#pragma once

#include <cstddef>
#include <tuple>

namespace yapt
{
    template <typename T, size_t N>
    class StaticSpan
    {
		T* p_data;

    public:
		typedef T* iterator;
		typedef const T* const_iterator;

		StaticSpan() = delete;

		constexpr
		StaticSpan(T& rhs) : p_data(&rhs) {}

		constexpr
		StaticSpan(const T& rhs) : p_data(const_cast<T*>(&rhs)) {}

        constexpr
        StaticSpan(T (&rhs) [N]) : p_data(const_cast<T*>(rhs)) {}

        constexpr
        StaticSpan(const T(&rhs) [N]) : p_data(const_cast<T*>(rhs)) {}

		constexpr iterator
		begin() const noexcept
		{ return p_data; }
		constexpr iterator
		end() const noexcept
		{ return p_data + N; }

		constexpr const_iterator
		cbegin() const noexcept
		{ return p_data; }
		constexpr const_iterator
		cend() const noexcept
		{ return p_data + N; }

		constexpr T&
		operator[](const size_t & i) noexcept
		{ return *(p_data + i); }

		constexpr const T&
		operator[](const size_t & i) const noexcept
		{ return *(p_data + i); }

		template <size_t I>
		constexpr T&
		get() noexcept
		{ return *(p_data + I); }

		template <size_t I>
		constexpr const T&
		get() const noexcept
		{ return *(p_data + I); }
    };
}

namespace std
{
	template <typename T, size_t N>
	class tuple_size<yapt::StaticSpan<T, N>>
    {
    public:
        static const constexpr size_t value = N;
    };

	template <size_t I, typename T, size_t N>
	class tuple_element<I, yapt::StaticSpan<T, N>>
	{
    public:
        using type = T;
    };

    template <size_t I, typename T, size_t N>
    constexpr decltype(auto)
    get(const yapt::StaticSpan<T, N>& span) noexcept
    { return span.template get<I>(); }
}