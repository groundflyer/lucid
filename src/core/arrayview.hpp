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
	T* _first;
    public:
	typedef T* iterator;
	typedef const T* const_iterator;

	constexpr
	ArrayView() = delete;

	constexpr explicit
	ArrayView(T* rhs) : _first(rhs) {}

	constexpr
	ArrayView(const ArrayView & rhs): _first(rhs._first) {}

	explicit constexpr
	ArrayView(const ArrayViewConst<T,N> & rhs): _first(rhs.cbegin()) {}

	constexpr ArrayView&
	operator=(const ArrayView& rhs) noexcept
	{
	    std::copy(rhs.cbegin(), rhs.cend(), _first);
	    return *this;
	}

	constexpr ArrayView&
	operator=(const ArrayViewConst<T,N>& rhs) noexcept
	{
	    std::copy(rhs.cbegin(), rhs.cend(), _first);
	    return *this;
	}

	constexpr iterator
	begin() noexcept
	{ return _first; }
	constexpr iterator
	end() noexcept
	{ return _first + N; }
	constexpr const_iterator
	begin() const noexcept
	{ return _first; }
	constexpr const_iterator
	end() const noexcept
	{ return _first + N; }
	constexpr const_iterator
	cbegin() const noexcept
	{ return _first; }
	constexpr const_iterator
	cend() const noexcept
	{ return _first + N; }

	constexpr T&
	operator[](const size_t & i) noexcept
	{ return *(_first + i); }
	constexpr const T&
	operator[](const size_t & i) const noexcept
	{ return *(_first + i); }
    };


    template <typename T, size_t N>
    class ArrayViewConst
    {
	const T* _first;
    public:
	typedef const T* const_iterator;
	typedef const_iterator iterator;

	constexpr
	ArrayViewConst() = delete;

	constexpr explicit
	ArrayViewConst(const T* rhs) : _first(rhs) {}

	constexpr
	ArrayViewConst(const ArrayViewConst & rhs): _first(rhs._first) {}

	constexpr explicit
	ArrayViewConst(const ArrayView<T,N> & rhs): _first(rhs.cbegin()) {}

	constexpr ArrayViewConst&
	operator=(const ArrayViewConst& rhs) = delete;

	constexpr const_iterator
	begin() const noexcept
	{ return _first; }
	constexpr const_iterator
	end() const noexcept
	{ return _first + N; }
	constexpr const_iterator
	cbegin() const noexcept
	{ return _first; }
	constexpr const_iterator
	cend() const noexcept
	{ return _first + N; }

	constexpr const T&
	operator[](const size_t & i) const noexcept
	{ return *(_first + i); }
    };
}
