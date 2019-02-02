// -*- C++ -*-
// mat_ops.hpp --
//

#pragma once

#include "math.hpp"
#include "debug.hpp"

#include <array>
#include <numeric>		// iota
#include <algorithm> 		// next_permutation


namespace yapt
{
    // forward declaration
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    class Matrix;

    template <typename T, size_t N,
			  template <typename, size_t> class Container>
    class Vector;

    // out-of-place matrix transposition
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    transpose(const Matrix<T, M, N, Container> & a) noexcept
    {
		Matrix<T, N, M, Container> ret;

		for (size_t i = 0; i < N; ++i)
			for(size_t j = 0; j < M; ++j)
				ret[i][j] = a[j][i];

		return ret;
    }


    // determinant
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr typename std::enable_if_t<M == N, T>
    det(const Matrix<T, M, N, Container> & a) noexcept
    {
		std::array<size_t, M> idxs {};
		std::iota(idxs.begin(), idxs.end(), 0);

		auto product = [&idxs, &a]()
			{
				T ret (1);
				for (size_t i = 0; i < M; ++i)
					ret *= a[i][idxs[i]];
				return ret;
			};

		auto get_elem = [&]()
			{ return math::sgn(idxs) * product(); };

		T ret = get_elem();

		const constexpr size_t rank = math::fac(M) - 1;
		for (size_t _ = 0; _ < rank; ++_)
		{
			std::next_permutation(idxs.begin(), idxs.end());
			ret += get_elem();
		}

		return ret;
    }

    // contructs minor matrix by removing I row, J column
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    minor_matrix(const Matrix<T, M, N, Container> & a,
				 const size_t I, const size_t J) noexcept
    {
		ASSERT(I < M || J < N, "Indicies out of range");

		Matrix<T, (M-1), (N-1), Container> ret;

		for (size_t i = 0; i < M-1; ++i)
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
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    cofactor(const Matrix<T, M, N, Container> & a) noexcept
    {
		Matrix<T, M, N, Container> ret;

		for (size_t i = 0; i < M; ++i)
			for(size_t j = 0; j < N; ++j)
				ret[i][j] = math::minus_one_pow(i+j) * det(minor_matrix(a, i, j));

		return ret;
    }

    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    is_invertible(const Matrix<T, M, N, Container> & a) noexcept
	{ return det(a) != 0; }

    // returns inverse matrix
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    inverse(const Matrix<T, M, N, Container> & a) noexcept
    {
		auto d = det(a);

		if (d != 0)
			return transpose(cofactor(a)) / d;
		else
			return a;
    }

    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class MContainer,
			  template <typename, size_t> class VContainer>
    constexpr auto
    dot(const Matrix<T, M, N, MContainer> & lhs,
		const Vector<T, N, VContainer> & rhs) noexcept
    {
		Vector<T, N, VContainer> ret;

		for (size_t i = 0; i < M; ++i)
			for (size_t j = 0; j < N; ++j)
				ret[i] += lhs.at(i, j) * rhs[j];

		return ret;
    }

    // Matrix-Matrix mupltiply
    template <typename T, size_t M1, size_t N1, size_t M2, size_t N2,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr typename std::enable_if_t<N1 == M2, Matrix<T, M1, N2, Container2>>
    dot(const Matrix<T, M1, N1, Container1> & lhs,
		const Matrix<T, M2, N2, Container2> & rhs) noexcept
    {
		Matrix<T, M1, N2, Container1> ret(0);

		for (size_t i = 0; i < M1; ++i)
			for (size_t j = 0; j < N2; ++j)
				for (size_t r = 0; r < N1; ++r)
					ret.at(i, j) += lhs.at(i, r) * rhs.at(r, j);

		return ret;
    }
}
