// -*- C++ -*-
// arrayview.hpp --
//

#pragma once


namespace yapt
{
    template <typename T, size_t N>
    class ArrayView
    {
		T* p_data = nullptr;

    public:
		typedef T* iterator;
		typedef const T* const_iterator;

		ArrayView() = delete;

		constexpr
		ArrayView(T* rhs) : p_data(rhs) {}

		constexpr
		ArrayView(const T* rhs) : p_data(const_cast<T*>(rhs)) {}

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
	class tuple_size<yapt::ArrayView<T, N>> : integral_constant<size_t, N> {};

	template <size_t I, typename T, size_t N>
	class tuple_element<I, yapt::ArrayView<T, N>>
	{ using type = T; };
}
