// -*- C++ -*-
// matrix.hpp --
//

#pragma once


#include "vec.hpp"
#include "mat_ops.hpp"
#include "arrayview.hpp"


namespace yapt
{
    template <typename T, size_t R, size_t C,
	      template <typename, size_t> class Container>
    class Matrix
    {
	static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");
	static_assert(R > 1 || C > 1, "One-dimensional matrices are not supported.");

	static const constexpr size_t N = R * C;

	using Data = Container<T, N>;
	Data _data {};

	static constexpr size_t
	pos(const size_t row, const size_t column)
	{ return row * C + column; }

	// variyng template sink
	constexpr
	void unpack(const size_t) {}

	// unpack vector arguments
	template <template <typename, size_t> class Container2, class ... Vectors>
	constexpr
	void unpack(size_t idx,
		    const Vector<T, C, Container2> & first,
		    const Vectors & ... vectors)
	{
	    std::copy(first.cbegin(), first.cend(), begin() + idx*C);
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
	template <typename T2, size_t R2, size_t C2,
		  template <typename, size_t> class Container2>
	explicit constexpr
	Matrix(const Matrix<T2, R2, C2, Container2> & rhs)
	{
	    for (size_t i = 0; i < std::min(R, R2); ++i)
		for (size_t j = 0; j < std::min(C, C2); ++j)
		    _data[pos(i, j)] = static_cast<T>(rhs[i][j]);
	}

	explicit constexpr
	Matrix(std::initializer_list<T> l)
	{ std::copy(l.begin(), l.begin()+std::min(l.size(), N), begin()); }

	// construct from a container
	// GCC 5.4.0 throws an internal compiler error, if we will use N instead of R*C
	template <template <typename, size_t> class Container2>
	explicit constexpr
	Matrix(const Container2<T, R*C> & rhs)
	{ std::copy(rhs.cbegin(), rhs.cend(), begin()); }

	// vector values constructor
	// only same dimensions vector are acceptable
	template <template <typename, size_t> class Container2, class ... Vectors>
	explicit constexpr
	Matrix(const Vector<T, C, Container2> & vector,
	       const Vectors & ... vectors)
	{
	    static_assert(sizeof...(vectors) <= R - 1,
			  "The number of elements doesn't match!");

	    unpack(0, vector, vectors...);
	}

	// scalar values constructor
	template <class ... Types>
	explicit constexpr
	Matrix(const T & first,
	       const Types & ... rest)
	{
	    static_assert(sizeof...(rest) <= N - 1,
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
	operator=(const Matrix<T, R, C, Container2> & rhs) noexcept
	{
	    std::copy(rhs.cbegin(), rhs.cend(), begin());
	    return *this;
	}

	constexpr Matrix&
	operator=(const T & rhs) noexcept
	{
	    for (T& i : _data) i = rhs;
	    return *this;
	}

	const constexpr Vector<T, C, ArrayViewConst>
	operator[](const size_t i) const noexcept
	{
	    ASSERT(i <= R, "Index out of range.");
	    return Vector<T, C, ArrayViewConst>(ArrayViewConst<T, C>(&(at(i,0))));
	}
	constexpr Vector<T, C, ArrayView>
	operator[](const size_t i) noexcept
	{
	    ASSERT(i <= R, "Index out of range.");
	    return Vector<T, C, ArrayView>(ArrayView<T, C>(&(at(i,0))));
	}

	const constexpr T&
	at(const size_t i) const noexcept
	{
	    ASSERT(i <= N, "Index out of range.");
	    return _data[i];
	}
	constexpr T&
	at(const size_t i) noexcept
	{
	    ASSERT(i <= N, "Index out of range.");
	    return _data[i];
	}

	const constexpr T&
	at(const size_t i, const size_t j) const noexcept
	{
	    ASSERT(i <= R && j <= C, "Index out of range.");
	    return _data[pos(i, j)];
	}
	constexpr T&
	at(const size_t i, const size_t j) noexcept
	{
	    ASSERT(i <= R && j <= C, "Index out of range.");
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
	operator+(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) + rhs.at(i);

	    return ret;
	}
	constexpr Matrix
	operator+(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) + rhs;

	    return ret;
	}
	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator+=(const Matrix<T, R, C, Container2> & rhs)
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) += rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator+=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) += rhs;

	    return *this;
	}


	template <template <typename, size_t> class Container2>
	constexpr Matrix
	operator-(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) - rhs.at(i);

	    return ret;
	}
	constexpr Matrix
	operator-(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) - rhs;

	    return ret;
	}
	constexpr Matrix
	operator-() const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = -at(i);

	    return ret;
	}
	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator-=(const Matrix<T, R, C, Container2> & rhs)
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) -= rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator-=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) -= rhs;

	    return *this;
	}

	// element-wise product
	template <template <typename, size_t> class Container2>
	constexpr Matrix
	operator*(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) * rhs.at(i);

	    return ret;
	}
	constexpr Matrix
	operator*(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) * rhs;

	    return ret;
	}
	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator*=(const Matrix<T, R, C, Container2> & rhs)
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) *= rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator*=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) *= rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr Matrix
	operator/(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) / rhs.at(i);

	    return ret;
	}
	constexpr Matrix
	operator/(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) / rhs;

	    return ret;
	}
	template <template <typename, size_t> class Container2>
	constexpr Matrix&
	operator/=(const Matrix<T, R, C, Container2> & rhs)
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) /= rhs.at(i);

	    return *this;
	}
	constexpr Matrix&
	operator/=(const T & rhs) noexcept
	{
	    for (size_t i = 0; i < N; ++i)
		at(i) /= rhs;

	    return *this;
	}

	template <template <typename, size_t> class Container2>
	constexpr auto
	operator==(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) == rhs.at(i);

	    return ret;
	}
	constexpr auto
	operator==(const T & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) == rhs;

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr auto
	operator!=(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) != rhs.at(i);

	    return ret;
	}
	constexpr auto
	operator!=(const T & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) != rhs;

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr auto
	operator<(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) < rhs.at(i);

	    return ret;
	}
	constexpr auto
	operator<(const T & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) < rhs;

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr auto
	operator<=(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) <= rhs.at(i);

	    return ret;
	}
	constexpr auto
	operator<=(const T & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) <= rhs;

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr auto
	operator>(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) > rhs.at(i);

	    return ret;
	}
	constexpr auto
	operator>(const T & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) > rhs;

	    return ret;
	}

	template <template <typename, size_t> class Container2>
	constexpr auto
	operator>=(const Matrix<T, R, C, Container2> & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) >= rhs.at(i);

	    return ret;
	}
	constexpr auto
	operator>=(const T & rhs)
	{
	    Matrix<bool, R, C, Container> ret;

	    for (size_t i = 0; i < N; ++i)
		ret.at(i) = at(i) >= rhs;

	    return ret;
	}


	template <typename Rhs>
	constexpr auto
	dot(Rhs && rhs) const noexcept
	{ return yapt::dot(*this, std::forward<Rhs>(rhs)); }


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

	    for (size_t i = 0; i < R; ++i)
		for (size_t j = 0; j < C; ++j)
		    ret.at(i, j) = i == j ? 1 : 0;

	    return ret;
	}


    	friend std::ostream&
    	operator<<(std::ostream & os, const Matrix & rhs)
    	{
    	    os << '[';
	    for (size_t i = 0; i < R; ++i)
	    {
		for (size_t j = 0; j < C; ++j)
		{
		    os << rhs.at(i, j);

		    if (i != R-1 || j != C-1)
		    	os << ',';
		    if (j < C - 1)
			os << '\t';
		}

		if (i < R - 1)
		    os << std::endl;
	    }
    	    os << ']';

    	    return os;
    	}
    };

    template <typename T, size_t R, size_t C,
	      template <typename, size_t> class Container>
    const constexpr size_t Matrix<T, R, C, Container>::N;
}
