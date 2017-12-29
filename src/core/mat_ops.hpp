// -*- C++ -*-
// mat_ops.hpp --
//

#pragma once

#include "misc.hpp"
#include "debug.hpp"

#include <array>
#include <numeric>		// iota
#include <algorithm> 		// next_permutation
#include <type_traits>


namespace yapt
{
    // forward declaration
    template <typename T, size_t C, size_t R,
			  template <typename, size_t> class Container>
    class Matrix;

    template <typename T, size_t N,
			  template <typename, size_t> class Container>
    class Vector;

    // out-of-place matrix transposition
    template <typename T, size_t R, size_t C,
			  template <typename, size_t> class Container>
    constexpr auto
    transpose(const Matrix<T, R, C, Container> & a) noexcept
    {
		Matrix<T, C, R, Container> ret;

		for (size_t i = 0; i < C; ++i)
			for(size_t j = 0; j < R; ++j)
				ret[i][j] = a[j][i];

		return ret;
    }


    // determinant
    template <typename T, size_t R, size_t C,
			  template <typename, size_t> class Container>
    constexpr typename std::enable_if_t<R == C, T>
    det(const Matrix<T, R, C, Container> & a) noexcept
    {
		std::array<size_t, R> idxs {};
		std::iota(idxs.begin(), idxs.end(), 0);

		auto product = [&idxs, &a]()
			{
				T ret (1);
				for (size_t i = 0; i < R; ++i)
					ret *= a[i][idxs[i]];
				return ret;
			};

		auto get_elem = [&]()
			{ return sgn(idxs) * product(); };

		T ret = get_elem();

		const constexpr size_t rank = fac(R) - 1;
		for (size_t _ = 0; _ < rank; ++_)
		{
			std::next_permutation(idxs.begin(), idxs.end());
			ret += get_elem();
		}

		return ret;
    }

    // contructs minor matrix by removing I row, J column
    template <typename T, size_t R, size_t C,
			  template <typename, size_t> class Container>
    constexpr auto
    minor_matrix(const Matrix<T, R, C, Container> & a,
				 const size_t I, const size_t J) noexcept
    {
		ASSERT(I < R || J < C, "Indicies out of range");

		Matrix<T, (R-1), (C-1), Container> ret;

		for (size_t i = 0; i < R-1; ++i)
			for (size_t j = 0; j < C-1; ++j)
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
    template <typename T, size_t R, size_t C,
			  template <typename, size_t> class Container>
    constexpr auto
    cofactor(const Matrix<T, R, C, Container> & a) noexcept
    {
		Matrix<T, R, C, Container> ret;

		for (size_t i = 0; i < R; ++i)
			for(size_t j = 0; j < C; ++j)
				ret[i][j] = minus_one_pow(i+j) * det(minor_matrix(a, i, j));

		return ret;
    }

    // returns inverse matrix if one exists else returs zeros matrix
    template <typename T, size_t R, size_t C,
			  template <typename, size_t> class Container>
    constexpr auto
    inverse(const Matrix<T, R, C, Container> & a) noexcept
    {
		Matrix<T, R, C, Container> ret;
		T d = det(a);

		ASSERT(d != 0, "The matrix is non-invertible.");

		if (d != static_cast<T>(0))
			ret = transpose(cofactor(a)) / d;

		return ret;
    }

    template <typename T, size_t R, size_t C,
			  template <typename, size_t> class MContainer,
			  template <typename, size_t> class VContainer>
    constexpr auto
    dot(const Matrix<T, R, C, MContainer> & lhs,
		const Vector<T, C, VContainer> & rhs) noexcept
    {
		Vector<T, C, VContainer> ret;

		for (size_t i = 0; i < R; ++i)
			for (size_t j = 0; j < C; ++j)
				ret[i] += lhs.at(i, j) * rhs[j];

		return ret;
    }

    // Matrix-Matrix mupltiply
    template <typename T, size_t R1, size_t C1,
			  size_t R2, size_t C2,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr typename std::enable_if_t<C1 == R2, Matrix<T, R1, C2, Container2>>
    dot(const Matrix<T, R1, C1, Container1> & lhs,
		const Matrix<T, R2, C2, Container2> & rhs) noexcept
    {
		Matrix<T, R1, C2, Container1> ret(0);

		for (size_t i = 0; i < R1; ++i)
			for (size_t j = 0; j < C2; ++j)
				for (size_t r = 0; r < C1; ++r)
					ret.at(i, j) += lhs.at(i, r) * rhs.at(r, j);

		return ret;
    }
}
