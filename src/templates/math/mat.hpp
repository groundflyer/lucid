// -*- C++ -*-
// matrix.hpp --
//

#pragma once


#include "vec.hpp"
#include "mat_ops.hpp"

#include <utils/arrayview.hpp>


namespace yapt
{
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    class Matrix
    {
	using Data = Container<T, N*N>;
	Data _data {};

	static constexpr size_t
	pos(const size_t row, const size_t column)
	{ return row * N + column; }

	// variyng template sink
	constexpr
	void unpack(const size_t) {}

	// unpack vector arguments
	template <template <typename, size_t> class Container2, class ... Vectors>
	constexpr
	void unpack(size_t idx,
		    const Vector<T, N, Container2> & first,
		    const Vectors & ... vectors)
	{
	    std::copy(first.cbegin(), first.cend(), _data.begin() + idx*N);
	    unpack(++idx, vectors...);
	}

	// unpack scalar arguments
	template <class ... Types>
	constexpr
	void unpack(size_t idx,
		    const T & first,
		    const Types & ... rest)
	{
	    _data[idx] = first;
	    unpack(++idx, rest...);
	}

    public:
	constexpr
    	Matrix() {}

	// single scalar construcor
    	explicit constexpr
    	Matrix(const T & rhs)
    	{ for (auto & elem : *this) elem = rhs;	}

    	explicit constexpr
    	Matrix(const Data & rhs) : _data(rhs) {}

	// conversion/copy constructor
	template <typename T2, size_t N2,
		  template <typename, size_t> class Container2>
	explicit constexpr
	Matrix(const Matrix<T2, N2, Container2> & rhs)
	{
	    const constexpr size_t minN = std::min(N, N2);
	    for (size_t i = 0; i < minN; ++i)
		for (size_t j = 0; j < minN; ++j)
		    _data[pos(i, j)] = static_cast<T>(rhs[i][j]);
	}

	explicit constexpr
	Matrix(std::initializer_list<T> l)
	{ std::copy(l.begin(), l.begin()+std::min(l.size(),N*N), _data.begin()); }

	// vector values constructor
	// only same dimensions vector are acceptable
	template <template <typename, size_t> class Container2, class ... Vectors>
	explicit constexpr
	Matrix(const Vector<T, N, Container2> & vector,
	       const Vectors & ... vectors)
	{
	    static_assert(sizeof...(vectors) <= N - 1,
			  "The number of elements doesn't match!");

	    unpack(0, vector, vectors...);
	}

	// scalar values constructor
	template <class ... Types>
	explicit constexpr
	Matrix(const T & first,
	       const Types & ... rest)
	{
	    static_assert(sizeof...(rest) <= N*N - 1,
			  "The number of elements doesn't match");

	    unpack(0, first, rest...);
	}

	constexpr Matrix&
	operator=(const Matrix& rhs) noexcept
	{
	    _data = rhs._data;
	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator=(const Matrix<T, N, Container2> & rhs) noexcept
	{
	    std::copy(rhs.cbegin(), rhs.cend(), _data.begin());
	    return *this;
	}

	constexpr Matrix&
	operator=(const T & rhs) noexcept
	{
	    for (T& i : _data) i = rhs;
	    return *this;
	}

	const constexpr Vector<T, N, ArrayViewConst>
	operator[](const size_t i) const noexcept
	{
	    ASSERT(i <= N, "Index out of range");
	    return Vector<T, N, ArrayViewConst>(ArrayViewConst<T, N>(&(at(i,0))));
	}
	constexpr Vector<T, N, ArrayView>
	operator[](const size_t i) noexcept
	{
	    ASSERT(i <= N, "Index out of range");
	    return Vector<T, N, ArrayView>(ArrayView<T, N>(&(at(i,0))));
	}

	const constexpr T&
	at(const size_t i) const noexcept
	{
	    ASSERT(i <= N*N, "Index out of range");
	    return _data[i];
	}
	constexpr T&
	at(const size_t i) noexcept
	{
	    ASSERT(i <= N*N, "Index out of range");
	    return _data[i];
	}

	const constexpr T&
	at(const size_t i, const size_t j) const noexcept
	{
	    ASSERT(i <= N && j <= N, "Index out of range");
	    return _data[pos(i, j)];
	}
	constexpr T&
	at(const size_t i, const size_t j) noexcept
	{
	    ASSERT(i <= N && j <= N, "Index out of range");
	    return _data[pos(i, j)];
	}

	constexpr typename Data::iterator
	begin() noexcept
	{ return _data.begin(); }
	constexpr typename Data::iterator
	end() noexcept
	{ return _data.end(); }

	constexpr typename Data::const_iterator
	begin() const noexcept
	{ return _data.begin(); }
	constexpr typename Data::const_iterator
	end() const noexcept
	{ return _data.end(); }

	constexpr decltype(auto)
	cbegin() const noexcept
	{ return _data.cbegin(); }
	constexpr decltype(auto)
	cend() const noexcept
	{ return _data.cend(); }

	template <template <typename, size_t> class Container2>
	constexpr Matrix
	operator+(const Matrix<T, N, Container2> & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = at(i) + rhs.at(i);

	    return ret;
	}
	constexpr Matrix
	operator+(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = at(i) + rhs;

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr Matrix
	operator-(const Matrix<T, N, Container2> & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = at(i) - rhs.at(i);

	    return ret;
	}
	constexpr Matrix
	operator-(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = at(i) - rhs;

	    return ret;
	}
	constexpr Matrix
	operator-() const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = -at(i);

	    return ret;
	}

	constexpr Matrix
	operator*(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = at(i) * rhs;

	    return ret;
	}

	constexpr Matrix
	operator/(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N*N; ++i)
		ret.at(i) = at(i) / rhs;

	    return ret;
	}

	// Matrix-Vector multiply
	template <template <typename, size_t> class Container2>
	constexpr Vector<T, N, Container>
	operator*(const Vector<T, N, Container2> & rhs) const noexcept
	{
	    Vector<T, N, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
		    ret[i] += at(i, j) * rhs[j];

	    return ret;
	}

	// // Matrix-Matrix mupltiply
	template <template <typename, size_t> class Container2>
	constexpr Matrix
	operator*(const Matrix<T, N, Container2> & rhs) const noexcept
	{
	    Matrix ret(0);

	    for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
		    for (size_t r = 0; r < N; ++r)
			ret.at(i, j) += at(i, r) * rhs.at(r, j);

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator+=(const Matrix<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) += rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator+=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) += rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator-=(const Matrix<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) -= rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator-=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) -= rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator*=(const Matrix<T, N, Container2> & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) *= rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator*=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) *= rhs;

	    return *this;
	}

	constexpr Matrix&
	operator/=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N*N; ++i)
		at(i) /= rhs;

	    return *this;
	}

	// transpose
	constexpr Matrix
	transpose() const noexcept
	{ return yapt::transpose(*this); }

	const constexpr Data&
	data() const noexcept
	{ return _data; }

	static constexpr Matrix
	unit()
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
		    ret.at(i,j) = i == j ? 1 : 0;

	    return ret;
	}


    	friend std::ostream&
    	operator<<(std::ostream & os, const Matrix & rhs)
    	{
    	    os << '[';
	    for (size_t i = 0; i < N; ++i)
		{
		    for (size_t j = 0; j < N; ++j)
			{
			    os << rhs.at(i, j);

			    if (j < N - 1)
				os << ',' << '\t';
			}

		    if (i < N - 1)
			os << std::endl;
		}
    	    os << ']';

    	    return os;
    	}
    };
}
