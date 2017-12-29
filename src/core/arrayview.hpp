// -*- C++ -*-
// arrayview.hpp --
//

#pragma once


namespace yapt
{
    // forward declaration
    template <typename T, size_t N>
    class ArrayViewConst;

    template <typename T, size_t N>
    class ArrayView
    {
		T* m_first = nullptr;

    public:
		typedef T* iterator;
		typedef const T* const_iterator;

		constexpr
		ArrayView() = delete;

		constexpr explicit
		ArrayView(T* rhs) : m_first(rhs) {}

		constexpr
		ArrayView(const ArrayView & rhs): m_first(rhs.m_first) {}

		explicit constexpr
		ArrayView(const ArrayViewConst<T, N> & rhs): m_first(rhs.cbegin()) {}

		constexpr ArrayView&
		operator=(const ArrayView& rhs) noexcept
		{
			std::copy(rhs.cbegin(), rhs.cend(), m_first);
			return *this;
		}

		constexpr ArrayView&
		operator=(const ArrayViewConst<T, N>& rhs) noexcept
		{
			std::copy(rhs.cbegin(), rhs.cend(), m_first);
			return *this;
		}

		constexpr iterator
		begin() noexcept
		{ return m_first; }
		constexpr iterator
		end() noexcept
		{ return m_first + N; }
		constexpr const_iterator
		begin() const noexcept
		{ return m_first; }
		constexpr const_iterator
		end() const noexcept
		{ return m_first + N; }
		constexpr const_iterator
		cbegin() const noexcept
		{ return m_first; }
		constexpr const_iterator
		cend() const noexcept
		{ return m_first + N; }

		constexpr T&
		operator[](const size_t & i) noexcept
		{ return *(m_first + i); }
		constexpr const T&
		operator[](const size_t & i) const noexcept
		{ return *(m_first + i); }

		template <size_t I>
		constexpr T&
		get() noexcept
		{ return *(m_first + I); }

		template <size_t I>
		const constexpr T&
		get() const noexcept
		{ return *(m_first + I); }
    };


    template <typename T, size_t N>
    class ArrayViewConst
    {
		const T* m_first = nullptr;

    public:
		typedef const T* const_iterator;
		typedef const_iterator iterator;

		constexpr
		ArrayViewConst() = delete;

		constexpr explicit
		ArrayViewConst(const T* rhs) : m_first(rhs) {}

		constexpr
		ArrayViewConst(const ArrayViewConst & rhs): m_first(rhs.m_first) {}

		constexpr explicit
		ArrayViewConst(const ArrayView<T, N> & rhs): m_first(rhs.cbegin()) {}

		constexpr ArrayViewConst&
		operator=(const ArrayViewConst& rhs) = delete;

		constexpr const_iterator
		begin() const noexcept
		{ return m_first; }
		constexpr const_iterator
		end() const noexcept
		{ return m_first + N; }
		constexpr const_iterator
		cbegin() const noexcept
		{ return m_first; }
		constexpr const_iterator
		cend() const noexcept
		{ return m_first + N; }

		constexpr const T&
		operator[](const size_t & i) const noexcept
		{ return *(m_first + i); }

		template <size_t I>
		const constexpr T&
		get() const noexcept
		{ return *(m_first + I); }
    };
}

namespace std
{
	template <typename T, size_t N>
	struct tuple_size<yapt::ArrayView<T, N>> : integral_constant<size_t, N> {};

	template <size_t I, typename T, size_t N>
	struct tuple_element<I, yapt::ArrayView<T, N>>
	{ using type = T; };

	template <typename T, size_t N>
	struct tuple_size<yapt::ArrayViewConst<T, N>> : integral_constant<size_t, N> {};

	template <size_t I, typename T, size_t N>
	struct tuple_element<I, yapt::ArrayViewConst<T, N>>
	{ using type = T; };
}
