// -*- C++ -*-
// mat_ops.hpp --
//

#pragma once

#include <templates/utils/debug.hpp>

#include <array>
#include <numeric>		// iota
#include <algorithm> 		// next_permutation


namespace yapt
{
    // forward declaration
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    class Matrix;

    // out-of-place matrix transposition
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    constexpr Matrix<T, N, Container>
    transpose(const Matrix<T, N, Container> & a) noexcept
    {
	Matrix<T, N, Container> ret;

	for (size_t i = 0; i < N; ++i)
	    for(size_t j = 0; j < N; ++j)
		ret[i][j] = a[j][i];

	return ret;
    }


    // determinant
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    constexpr T
    det(const Matrix<T, N, Container> & a) noexcept
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
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    constexpr Matrix<T, (N-1), Container>
    minor_matrix(const Matrix<T, N, Container> & a,
		 const size_t I, const size_t J) noexcept
    {
	ASSERT(I < N || J < N, "Indicies out of range");

	Matrix<T, (N-1), Container> ret;

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
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    constexpr Matrix<T, N, Container>
    cofactor(const Matrix<T, N, Container> & a) noexcept
    {
	Matrix<T, N, Container> ret;

	for (size_t i = 0; i < N; ++i)
	    for(size_t j = 0; j < N; ++j)
		ret[i][j] = minus_one_pow(i+j) * det(minor_matrix(a, i, j));

	return ret;
    }

    // returns inverse matrix if one exists else returs zeros matrix
    template <typename T, size_t N,
	      template <typename, size_t> class Container>
    constexpr Matrix<T, N, Container>
    inverse(const Matrix<T, N, Container> & a) noexcept
    {
	Matrix<T, N, Container> ret;
	T d = det(a);

	if (d != static_cast<T>(0))
	    {
		T _d = static_cast<T>(1) / d;
		ret = transpose(cofactor(a)) * _d;
	    }

	return ret;
    }
}
