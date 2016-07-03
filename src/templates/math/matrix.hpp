// -*- C++ -*-
// matrix.hpp --
//

#pragma once


#include <math/vec.hpp>

#include <algorithm> 		// next_permutation
#include <numeric>		// iota


namespace yapt
{
    template <typename T, size_t N>
    class Matrix
    {
	using Data = std::array<std::array<T, N>, N>;

	// unit matrix generator
	static const constexpr
	Data unit()
	{
	    Data ret {};

	    for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
		    if (i == j)
			ret[i][j] = 1;
		    else
			ret[i][j] = 0;

	    return ret;
	}

	// variyng template sink
	constexpr
	void unpack(const size_t &)
	{ }

	// unpack vector arguments
	template <class ... Vectors>
	constexpr
	void unpack(size_t idx,
		    const Vector<T, N> & first,
		    const Vectors & ... vectors)
	{
	    _data[idx++] = first.data();

	    if (sizeof...(vectors) > 0)
		unpack(idx, vectors...);
	}

	// unpack scalar arguments
	template <class ... Types>
	constexpr
	void unpack(size_t idx,
		    const T & first,
		    const Types & ... rest)
	{
	    _data[idx/N][idx%N] = first;

	    if (sizeof...(rest) > 0)
		unpack(++idx, rest...);
	}

	// the default matrix is a unit matrix
	Data _data = unit();

    public:
	constexpr
    	Matrix() { }

	// single scalar construcor
    	explicit constexpr
    	Matrix(const T & rhs)
    	{
	    for (auto & elem : *this)
		elem = rhs;
	}

    	explicit constexpr
    	Matrix(const int & rhs)
    	{
	    for (auto & elem : *this)
		elem = T(rhs);
	}

    	explicit constexpr
    	Matrix(const Data & rhs) : _data(rhs) { }

	// helper constructor for N-1 sized matrices
	explicit constexpr
	Matrix(const Matrix<T, (N-1)> & rhs)
	{
	    for (size_t i = 0; i < N-1; ++i)
		for (size_t j = 0; j < N-1; ++j)
		    _data[i][j] = rhs[i][j];
	}

	// helper constructor for arrays
	explicit constexpr
	Matrix(const std::array<T, N*N> & rhs)
	{ std::copy(rhs.begin(), rhs.end(), this->begin()); }

	// unsafe helper constructor for c-arrays
	explicit constexpr
	Matrix(T * rhs)
	{ std::copy(rhs, rhs+N*N, this->begin()); }

	explicit constexpr
	Matrix(std::initializer_list<T> l)
	{
	    std::copy(l.begin(), l.begin()+std::min(l.size(),N*N),
		      this->begin());
	}

	// vector values constructor
	// only same dimensions vector are acceptable
	template <class ... Vectors> explicit constexpr
	Matrix(const Vector<T, N> & vector,
	       const Vectors & ... vectors)
	{
	    static_assert(sizeof...(vectors) == N - 1,
			  "The number of elements doesn't match!");

	    unpack(0, vector, vectors...);
	}

	// scalar values constructor
	template <class ... Types> explicit constexpr
	Matrix(const T & first,
	       const Types & ... rest)
	{
	    static_assert(sizeof...(rest) == N*N - 1,
			  "The number of elements doesn't match!");

	    unpack(0, first, rest...);
	}

	// is invertible
	explicit constexpr
	operator bool() const noexcept
	{ return det(*this) != 0; }

	template <typename T2>
	explicit constexpr
	operator Matrix<T2, N>() const noexcept
	{
	    Matrix<T2, N> ret;
	    std::copy(this->cbegin(), this->cend(), ret.begin());
	    return ret;
	}

	const constexpr auto &
	operator[](const size_t & i) const noexcept
	{
	    ASSERT(i < N);
	    return _data[i];
	}

	constexpr auto &
	operator[](const size_t & i) noexcept
	{
	    ASSERT(i < N);
	    return _data[i];
	}

	constexpr T*
	begin() noexcept
	{ return _data.begin()->begin(); }
	constexpr T*
	end() noexcept
	{ return _data.end()->end() - N; }

	const constexpr T*
	cbegin() const noexcept
	{ return _data.cbegin()->cbegin(); }
	const constexpr T*
	cend() const noexcept
	{ return _data.cend()->cend() - N; }

	constexpr Matrix
	operator+(const Matrix & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = _data[i][j] + rhs._data[i][j];

	    return ret;
	}
	constexpr Matrix
	operator+(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = _data[i][j] + rhs;

	    return ret;
	}

	constexpr Matrix
	operator-(const Matrix & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = _data[i][j] - rhs._data[i][j];

	    return ret;
	}
	constexpr Matrix
	operator-(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = _data[i][j] - rhs;

	    return ret;
	}
	constexpr Matrix
	operator-() const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = -_data[i][j];

	    return ret;
	}

	constexpr Matrix
	operator*(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = _data[i][j] * rhs;

	    return ret;
	}

	// Matrix-Vector multiply
	constexpr Vector<T, N>
	operator*(const Vector<T, N> & rhs) const noexcept
	{
	    Vector<T, N> ret;

	    for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
		    ret[i] += _data[i][j] * rhs[j];

	    return ret;
	}

	// Matrix-Matrix mupltiply
	constexpr Matrix
	operator*(const Matrix & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < N; ++j)
		    {
			ret._data[i][j] = 0;

			for (size_t r = 0; r < N; ++r)
			    ret._data[i][j] += _data[i][r] * rhs[r][j];
		    }

	    return ret;
	}

	constexpr Matrix
	operator/(const T & rhs) const noexcept
	{
	    Matrix ret;

	    for (size_t i = 0; i < N; ++i)
		for(size_t j = 0; j < N; ++j)
		    ret._data[i][j] = _data[i][j] / rhs;

	    return ret;
	}

	constexpr Matrix&
	operator+=(const Matrix & rhs) noexcept
	{ *this = *this + rhs; return *this; }
	constexpr Matrix&
	operator+=(const T & rhs) noexcept
	{ *this = *this + rhs; return *this; }

	constexpr Matrix&
	operator-=(const Matrix & rhs) noexcept
	{ *this = *this + rhs; return *this; }
	constexpr Matrix&
	operator-=(const T & rhs) noexcept
	{ *this = *this + rhs; return *this; }

	constexpr Matrix&
	operator*=(const Matrix & rhs) noexcept
	{ *this = *this * rhs; return *this; }
	constexpr Matrix&
	operator*=(const T & rhs) noexcept
	{ *this = *this * rhs; return *this; }

	constexpr Matrix&
	operator/=(const T & rhs) noexcept
	{ *this = *this / rhs; return *this; }

	// in-place transpose
	constexpr void
	transpose() noexcept
	{
	    for (size_t i = 0; i < N-1; ++i)
	    	for (size_t j = i+1; j < N; ++j)
	    	    if (i != j)
	    		std::swap(_data[i][j], _data[j][i]);
	}

	inline const constexpr Data&
	data() const noexcept
	{ return _data; }


    	friend std::ostream&
    	operator<<(std::ostream & os, const Matrix & rhs)
    	{
    	    os << "[";
	    for (size_t i = 0; i < N; ++i)
		{
		    for (size_t j = 0; j < N; ++j)
			{
			    os << rhs._data[i][j];

			    if (j < N - 1)
				os << ",\t";
			}

		    if (i < N - 1)
			os << std::endl;
		}
    	    os << "]";

    	    return os;
    	}
    };


    // out-of-place matrix transposition routine
    template <typename T, size_t N>    
    constexpr Matrix<T, N>
    transpose(const Matrix<T, N> & a) noexcept
    {
	Matrix<T, N> ret;

	for (size_t i = 0; i < N; ++i)
	    for(size_t j = 0; j < N; ++j)
		ret[i][j] = a[j][i];

	return ret;
    }


    // determinant routine
    template <typename T, size_t N>    
    constexpr T
    det(const Matrix<T, N> & a) noexcept
    {
	std::array<size_t, N> idxs {};
	std::iota(idxs.begin(), idxs.end(), 0);

	auto product = [&idxs, &a]()
	    {
		T ret (1);
		for (size_t i = 0; i < N; ++i)
		    ret *= a[i][idxs[i]];
		return ret;
	    };

	auto get_elem = [&]()
	{ return sgn(idxs) * product(); };

	const constexpr size_t rank = fac(N) - 1;
	T ret = get_elem();

	for (size_t _ = 0; _ < rank; ++_)
	    {
		std::next_permutation(idxs.begin(), idxs.end());
		ret += get_elem();
	    }

	return ret;
    }

    // contructs minor matrix by removing I row, J column
    template <typename T, size_t N>
    constexpr Matrix<T, (N-1)>
    minor_matrix(const Matrix<T, N> & a,
		 const size_t & I, const size_t & J) noexcept
    {
	ASSERT(I < N || J < N);

	Matrix<T, (N-1)> ret;

	for (size_t i = 0; i < N-1; ++i)
	    for (size_t j = 0; j < N-1; ++j)
		    {
			size_t idx = i;
			size_t jdx = j;

			if (i >= I)
			    idx = i + 1;

			if (j >= J)
			    jdx = j + 1;

			ret[i][j] = a[idx][jdx];
		    }

	return ret;
    }

    // cofactor matrix
    template <typename T, size_t N>
    constexpr Matrix<T, N>
    cofactor(const Matrix<T, N> & a) noexcept
    {
	Matrix<T, N> ret;

	for (size_t i = 0; i < N; ++i)
	    for(size_t j = 0; j < N; ++j)
		ret[i][j] = minus_one_pow(i+j) * det(minor_matrix(a, i, j));

	return ret;
    }

    // returns inverse matrix if one exists else returs zeros matrix
    template <typename T, size_t N>
    constexpr Matrix<T, N>
    inverse(const Matrix<T, N> & a) noexcept
    {
	Matrix<T, N> ret(T(0));
	T d = det(a);

	if (d != 0)
	    {
		T _d = 1 / d;
		ret = transpose(cofactor(a)) * _d;
	    }	    

	return ret;
    }
}
