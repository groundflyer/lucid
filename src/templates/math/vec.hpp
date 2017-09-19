// -*- C++ -*-
// vec.hpp --
//

#pragma once

#include "vec_ops.hpp"
#include <templates/utils/debug.hpp>

#include <iostream>		// operator <<
#include <initializer_list>


namespace yapt
{
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    class Vector
    {
	using Data = Container<T,N>;

    protected:
	Data _data {};

    public:
	constexpr
	Vector() {}

	constexpr
	Vector(const Vector & rhs) : _data(rhs._data) {}

	explicit constexpr
	Vector(const Data & rhs) : _data(rhs) {}

	explicit constexpr
	Vector(const T & rhs) { for (T& i : _data) i = rhs; }

	constexpr
	Vector(Vector && rhs) : _data(std::forward<Data>(rhs._data)) {}

	explicit constexpr
	Vector(Data && rhs) : _data(std::forward<Data>(rhs)) {}

	explicit constexpr
	Vector(T && rhs) { for (T& i : _data) i = rhs; }

	explicit constexpr
	Vector(std::initializer_list<T> l)
	{ std::copy(l.begin(), l.begin()+std::min(l.size(), N), _data.begin()); }

	template <class ... Types>
	explicit constexpr
	Vector(const T & first,
	       const Types & ... args) : _data({{first, args...}})
	{
	    static_assert(sizeof...(args) <= N - 1,
	    		  "The number of elements doesn't match!");
	}

	template <class ... Types>
	explicit constexpr
	Vector(T && first,
	       Types && ... args) :
	_data({{std::forward<T>(first), (std::forward<T>(args))...}})
	{
	    static_assert(sizeof...(args) <= N - 1,
	    		  "The number of elements doesn't match!");
	}

	// conversion constructor
	template <typename T2, size_t N2,
		  template <typename, size_t> class Container2>
	explicit constexpr
	Vector(const Vector<T2, N2, Container2> & rhs)
	{ for (size_t i = 0; i < std::min(N, N2); ++i) _data[i] = static_cast<T>(rhs[i]); }

	constexpr Vector&
	operator=(const Vector & rhs) noexcept
	{
	    _data = rhs._data;
	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Vector&
	operator=(const Vector<T, N, Container2> & rhs) noexcept
	{
	    std::copy(rhs.cbegin(), rhs.cend(), _data.begin());
	    return *this;
	}

	constexpr Vector&
	operator=(const T & rhs) noexcept
	{
	    for (T& i : _data) i = rhs;
	    return *this;
	}

	constexpr auto
	begin() noexcept
	{ return _data.begin(); }
	constexpr auto
	end() noexcept
	{ return _data.end(); }
	constexpr auto
	cbegin() const noexcept
	{ return _data.cbegin(); }
	constexpr auto
	cend() const noexcept
	{ return _data.cend(); }

	const constexpr T&
	operator[](const size_t i) const noexcept
	{
	    ASSERT(i <= N, "Index out of range");
	    return _data[i];
	}
	constexpr T&
	operator[](const size_t i) noexcept
	{
	    ASSERT(i <= N, "Index out of range");
	    return _data[i];
	}

	const constexpr T&
	at(const size_t i) const noexcept
	{
	    ASSERT(i <= N, "Index out of range");
	    return _data[i];
	}
	constexpr T&
	at(const size_t i) noexcept
	{
	    ASSERT(i <= N, "Index out of range");
	    return _data[i];
	}

	const constexpr Data&
	data() const noexcept
	{ return _data; }

	template <template <typename, size_t> class Container2>
	constexpr Vector
	operator+(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::plus<T>()); }
	constexpr Vector
	operator+(const T & rhs) const noexcept
	{ return transform(*this, [&rhs](const T& a){ return a + rhs; }); }

	template <template <typename, size_t> class Container2>
	constexpr Vector
	operator-(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::minus<T>()); }
	constexpr Vector
	operator-(const T & rhs) const noexcept
	{ return transform(*this, [&rhs](const T& a){ return a - rhs; }); }
	constexpr Vector
	operator-() const noexcept
	{ return transform(*this, std::negate<T>()); }

	template <template <typename, size_t> class Container2>
	constexpr Vector
	operator*(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::multiplies<T>()); }
	constexpr Vector
	operator*(const T & rhs) const noexcept
	{ return transform(*this, [&rhs](const T& a){ return a * rhs; }); }

	template <template <typename, size_t> class Container2>
	constexpr Vector
	operator/(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::divides<T>()); }
	constexpr Vector
	operator/(const T & rhs) const noexcept
	{ return transform(*this, [&rhs](const T& a){ return a / rhs; }); }

	template <template <typename, size_t> class Container2>
	constexpr Vector&
	operator+=(const Vector<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] += rhs[i];

	    return *this;
	}
	constexpr Vector&
	operator+=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] += rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Vector&
	operator-=(const Vector<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] -= rhs[i];

	    return *this;
	}
	constexpr Vector&
	operator-=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] -= rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Vector&
	operator*=(const Vector<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] *= rhs[i];

	    return *this;
	}
	constexpr Vector&
	operator*=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] *= rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Vector&
	operator/=(const Vector<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] /= rhs[i];

	    return *this;
	}
	constexpr Vector&
	operator/=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		_data[i] /= rhs;

	    return *this;
	}


	template <template <typename, size_t> class Container2>
	constexpr Vector<bool, N, Container>
	operator==(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::equal_to<T>()); }

	template <template <typename, size_t> class Container2>
	constexpr Vector<bool, N, Container>
	operator!=(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::not_equal_to<T>()); }

	template <template <typename, size_t> class Container2>
	constexpr Vector<bool, N, Container>
	operator>(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::greater<T>()); }

	template <template <typename, size_t> class Container2>
	constexpr Vector<bool, N, Container>
	operator<(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::less<T>()); }

	template <template <typename, size_t> class Container2>
	constexpr Vector<bool, N, Container>
	operator>=(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::greater_equal<T>()); }

	template <template <typename, size_t> class Container2>
	constexpr Vector<bool, N, Container>
	operator<=(const Vector<T, N, Container2> & rhs) const noexcept
	{ return transform(*this, rhs, std::less_equal<T>()); }


	template <template <typename, size_t> class Container2>
	constexpr T
	dot(const Vector<T, N, Container2> & rhs) const noexcept
	{ return yapt::dot(*this, rhs); }

	template <template <typename, size_t> class Container2>
	constexpr Vector
	cross(const Vector<T, N, Container2> & rhs) const noexcept
	{ return yapt::cross(*this, rhs); }

	constexpr T
	length() const noexcept
	{ return yapt::length(*this); }

	constexpr size_t
	size() const noexcept
	{ return N; }

	friend std::ostream&
	operator<<(std::ostream & os, const Vector & rhs) noexcept
	{
	    os << '[';
	    for (size_t i = 0; i < N; ++i)
		{
		    os << rhs._data[i];

		    if (i < N - 1)
			os << ',' << '\t';
		}
	    os << ']';

	    return os;
	}
    };
}
