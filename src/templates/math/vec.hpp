// -*- C++ -*-
// vec.hpp --
//

#pragma once

#include <array>
#include <functional>
#include <utility>
#include <cmath> 		// sqrt
#include <iostream>		// operator <<
#include <initializer_list>
#include <algorithm>		// copy


#include <utils/iter_small.hpp>
#include <utils/misc.hpp>
#include <utils/debug.hpp>



namespace yapt
{
    template <typename T, size_t N>
    class Vector
    {
	using Data = std::array<T, N>;

	Data _data {};

    public:
	constexpr
	Vector() { }

	explicit constexpr
	Vector(const Data & rhs) : _data(rhs) { }

	explicit constexpr
	Vector(const T & rhs) { for (T& i : _data) i = rhs; }

	explicit constexpr
	Vector(std::initializer_list<T> l)
	{ std::copy(l.begin(), l.begin()+std::min(l.size(), N), _data.begin()); }

	template <class ... Types> explicit constexpr
	Vector(const T && first,
	       const Types && ... args) :
	    _data({{std::forward<const T>(first), (std::forward<const T>(args))...}})
	{
	    static_assert(sizeof...(args) <= N - 1,
	    		  "The number of elements doesn't match!");
	}

	// same type another dim
	template <size_t N2>
	explicit constexpr
	Vector(const Vector<T, N2> & rhs)
	{ for (size_t i = 0; i < std::min(N, N2); i++) _data[i] = rhs[i]; }

	// bool converter
	explicit constexpr
	operator bool() const noexcept
	{ return length(*this) > static_cast<T>(0); }

	// same dim, other type converter
	template <typename T2>
	explicit constexpr
	operator Vector<T2, N>() const noexcept
	{
	    Vector<T2, N> ret;

	    for (size_t i = 0; i < N; ++i)
		ret[i] = static_cast<T>(_data[i]);

	    return ret;
	}

	constexpr typename Data::iterator
	begin() noexcept
	{ return _data.begin(); }
	constexpr typename Data::iterator
	end() noexcept
	{ return _data.end(); }
	const constexpr typename Data::const_iterator
	cbegin() const noexcept
	{ return _data.cbegin(); }
	const constexpr typename Data::const_iterator
	cend() const noexcept
	{ return _data.cend(); }


	constexpr Vector&
	operator=(const T & rhs)
	{
	    for (T& i : _data) i = rhs;
	    return *this;
	}

	const constexpr T&
	operator[](const size_t & i) const noexcept
	{
	    ASSERT(i < N);
	    return _data[i];
	}
	constexpr T&
	operator[](const size_t & i) noexcept
	{
	    ASSERT(i < N);
	    return _data[i];
	}


	constexpr Vector
	operator+(const Vector & rhs) const noexcept
	{ return bmap(std::plus<T>(), *this, rhs); }
	constexpr Vector
	operator+(const T & rhs) const noexcept
	{ return bmap(std::plus<T>(), *this, rhs); }

	constexpr Vector
	operator-(const Vector & rhs) const noexcept
	{ return bmap(std::minus<T>(), *this, rhs); }
	constexpr Vector
	operator-(const T & rhs) const noexcept
	{ return bmap(std::minus<T>(), *this, rhs); }
	constexpr Vector
	operator-() const noexcept
	{ return umap(std::negate<T>(), *this); }

	constexpr Vector
	operator*(const Vector & rhs) const noexcept
	{ return bmap(std::multiplies<T>(), *this, rhs); }
	constexpr Vector
	operator*(const T & rhs) const noexcept
	{ return bmap(std::multiplies<T>(), *this, rhs); }

	constexpr Vector
	operator/(const Vector & rhs) const noexcept
	{ return bmap(std::divides<T>(), *this, rhs); }
	constexpr Vector
	operator/(const T & rhs) const noexcept
	{ return bmap(std::divides<T>(), *this, rhs); }

	constexpr T
	operator%(const Vector & rhs) const noexcept
	{ return dot(*this, rhs); }

	constexpr Vector
	operator^(const Vector & rhs) const noexcept
	{ return cross(*this, rhs); }

	constexpr Vector&
	operator+=(const Vector & rhs) noexcept
	{ *this = *this + rhs; return *this; }
	constexpr Vector&
	operator+=(const T & rhs) noexcept
	{ *this = *this + rhs; return *this; }

	constexpr Vector&
	operator-=(const Vector & rhs) noexcept
	{ *this = *this - rhs; return *this; }
	constexpr Vector&
	operator-=(const T & rhs) noexcept
	{ *this = *this - rhs; return *this; }

	constexpr Vector&
	operator*=(const Vector & rhs) noexcept
	{ *this = *this * rhs; return *this; }
	constexpr Vector&
	operator*=(const T & rhs) noexcept
	{ *this = *this * rhs; return *this; }

	constexpr Vector&
	operator/=(const Vector & rhs) noexcept
	{ *this = *this / rhs; return *this; }
	constexpr Vector&
	operator/=(const T & rhs) noexcept
	{ *this = *this / rhs; return *this; }

	constexpr bool
	operator==(const Vector & rhs) const noexcept
	{ return _data == rhs._data; }
	constexpr bool
	operator!=(const Vector & rhs) const noexcept
	{ return _data != rhs._data; }

	constexpr bool
	operator>(const Vector & rhs) const noexcept
	{ return reduce(std::logical_and<T>(),
			bmap(std::greater<T>(), *this, rhs)); }
	constexpr bool
	operator<(const Vector & rhs) const noexcept
	{ return reduce(std::logical_and<T>(),
			bmap(std::less<T>(), *this, rhs)); }
	constexpr bool
	operator>=(const Vector & rhs) const noexcept
	{ return reduce(std::logical_and<T>(),
			bmap(std::greater_equal<T>(), *this, rhs)); }
	constexpr bool
	operator<=(const Vector & rhs) const noexcept
	{ return reduce(std::logical_and<T>(),
			bmap(std::less_equal<T>(), *this, rhs)); }

	static constexpr size_t
	size() noexcept
	{ return N; }

	const constexpr Data&
	data() const noexcept
	{ return _data; }

	constexpr void
	normalize() noexcept
	{
	    T l = length(*this);

	    if (l != static_cast<T>(1) || l != static_cast<T>(0))
		for (T & elem : _data)
		    elem /= l;
	}

	// shortcuts
	const constexpr T&
	u() const noexcept
	{ return _data[0]; }
	constexpr T&
	u() noexcept
	{ return _data[0]; }

	const constexpr T&
	v() const noexcept
	{ return _data[1]; }
	constexpr T&
	v() noexcept
	{ return _data[1]; }

	const constexpr T&
	x() const noexcept
	{ return _data[0]; }
	constexpr T&
	x() noexcept
	{ return _data[0]; }

	const constexpr T&
	y() const noexcept
	{ return _data[1]; }
	constexpr T&
	y() noexcept
	{ return _data[1]; }

	const constexpr T&
	z() const noexcept
	{
	    static_assert(N <= 3, "It doesn't have third element!");
	    return _data[2];
	}
	constexpr T&
	z() noexcept
	{
	    static_assert(N <= 3, "It doesn't have third element!");
	    return _data[2];
	}

	const constexpr T&
	w() const noexcept
	{
	    static_assert(N <= 4, "It doesn't have fourth element!");
	    return _data[3];
	}
	constexpr T&
	w() noexcept
	{
	    static_assert(N <= 4, "It doesn't have fourth element!");
	    return _data[3];
	}

	const constexpr T&
	r() const noexcept
	{ return _data[0]; }
	constexpr T&
	r() noexcept
	{ return _data[0]; }

	const constexpr T&
	g() const noexcept
	{ return _data[1]; }
	constexpr T&
	g() noexcept
	{ return _data[1]; }

	const constexpr T&
	b() const noexcept
	{
	    static_assert(N <= 3, "It doesn't have a third element!");
	    return _data[2];
	}
	constexpr T&
	b() noexcept
	{
	    static_assert(N <= 3, "It doesn't have a third element!");
	    return _data[2];
	}

	const constexpr T&
	a() const noexcept
	{
	    static_assert(N <= 4, "It doesn't have an alpha channel!");
	    return _data[3];
	}
	constexpr T&
	a() noexcept
	{
	    static_assert(N <= 4, "It doesn't have an alpha channel!");
	    return _data[3];
	}


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


    // dot product
    template <typename T, size_t N,
    	      template <typename, size_t> class Vector>
    constexpr T
    dot(const Vector<T, N> & a,
	const Vector<T, N> & b) noexcept
    { return reduce(std::plus<T>(), bmap(std::multiplies<T>(), a, b)); }


    // N-dimensional cross product
    template <typename T, size_t N,
	      template <typename, size_t> class Vector>
    constexpr Vector<T, N>
    cross(const Vector<T, N> & a,
	  const Vector<T, N> & b) noexcept
    {
	Vector<T, N> ret;

	for (size_t i = 0; i < N; ++i)
	    for (size_t j = 0; j < N; ++j)
		for (size_t k = 0; k < N; ++k)
		    ret[i] += sgn(std::array<size_t, 3>({i,j,k})) * a[j] * b[k];

	return ret;
    }

    // 3-dimensional cross product
    template <typename T,
	      template <typename, size_t> class Vector>
    constexpr Vector<T, 3>
    cross(const Vector<T, 3> & a,
	  const Vector<T, 3> & b) noexcept
    {
	Vector<T, 3> ret;

// #ifdef LEFT			// Lefthanded coordinate system
// 	ret.x() = a.z()*b.y() - a.y()*b.z();
// 	ret.y() = a.x()*b.z() - a.z()*b.x();
// 	ret.z() = a.y()*b.x() - a.x()*b.y();
// #else  // Righthanded
	ret.x() = a.y()*b.z() - a.z()*b.y();
	ret.y() = a.z()*b.x() - a.x()*b.z();
	ret.z() = a.x()*b.y() - a.y()*b.x();
// #endif	// LEFT

	return ret;
    }


    template <typename T, size_t N>
    constexpr T
    length(const Vector<T, N> & a) noexcept
    { return std::sqrt(reduce(std::plus<T>(), umap(pow<T, 2>, a))); }

    template <typename T, size_t N>
    constexpr T
    length2(const Vector<T, N> & a) noexcept
    { return reduce(std::plus<T>(), umap(pow<T, 2>, a)); }


    template <typename T, size_t N,
	      template <typename, size_t> class Vector>
    constexpr Vector<T, N>
    normalize(Vector<T, N> a) noexcept
    {
	T l = length(a);

	if (l != static_cast<T>(1) || l != static_cast<T>(0))
	    a /= l;

	return a;
    }

    template <typename T, size_t N>
    constexpr T
    sum(const Vector<T, N> & a) noexcept
    { return reduce(std::plus<T>(), a); }

    template <typename T, size_t N>
    constexpr T
    avg(const Vector<T, N> & a) noexcept
    { return sum(a) / static_cast<T>(N); }

    template <typename T, size_t N>
    constexpr T
    max(const Vector<T, N> & a) noexcept
    { return reduce(max<T>, a); }

    template <typename T, size_t N>
    constexpr T
    min(const Vector<T, N> & a) noexcept
    { return reduce(min<T>, a); }
}
