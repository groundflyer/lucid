// -*- C++ -*-
// matrix.hpp --
//

#pragma once


#include "vector.hpp"


namespace lucid
{
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container = std::array>
    class Matrix;

    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class MContainer,
			  template <typename, size_t> class VContainer>
    constexpr auto
    dot(const Matrix<T, M, N, MContainer> & lhs,
		const Vector<T, N, VContainer> & rhs) noexcept
    {
		Vector<T, N> ret;

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
		Matrix<T, M1, N2> ret(0);

		for (size_t i = 0; i < M1; ++i)
			for (size_t j = 0; j < N2; ++j)
				for (size_t r = 0; r < N1; ++r)
					ret.at(i, j) += lhs.at(i, r) * rhs.at(r, j);

		return ret;
    }

    template <typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
    class Matrix
    {
		static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");
		static_assert(M > 1 && N > 1, "One-dimensional matrices are not supported.");

		static const constexpr size_t MN = M * N;

		using Data = Container<T, MN>;
		Data m_data {};

		static constexpr auto
		pos(const size_t i, const size_t j, const size_t J = N) noexcept
		{ return i * J + j; }

        static constexpr auto
        indices(const size_t pos) noexcept
        { return std::pair(pos / N, pos % N); }

		// variyng template sink
        template <size_t>
		constexpr void
        unpack() const {}

        template <size_t idx, typename ... Ts>
        constexpr void
        unpack(const T& first, Ts && ... other) noexcept
        {
            static_assert(idx < MN, "Too many elements.");
            m_data[idx] = first;
            if constexpr (sizeof...(other) == 0 && idx < MN-1)
                unpack<idx+1>(first);
            else
                unpack<idx+1>(std::forward<Ts>(other)...);
        }

		// unpack vector arguments
		template <size_t idx, size_t MN1,
                  template <typename, size_t> typename Container2,
				  typename ... Ts>
		constexpr void
        unpack(const Vector<T, MN1, Container2>& first,
               Ts&& ... other) noexcept
		{
            static_assert(idx < MN, "Too many elements.");
            const constexpr auto end = std::min(MN - idx, MN1);
            for(size_t i = 0; i < end; ++i)
                m_data[idx + i] = first[i];
			unpack<idx + end>(std::forward<Ts>(other)...);
		}

        template <size_t idx, size_t MN1,
                  typename ... Ts>
        constexpr void
        unpack(const Container<T, MN1>& first, Ts && ... other) noexcept
        {
            static_assert(idx < MN, "Too many elements.");
            const constexpr auto end = std::min(MN - idx, MN1);
            for(size_t i = 0; i < end; ++i)
                m_data[idx + i] = first[i];
            unpack<idx + end>(std::forward<Ts>(other)...);
        }

		template <size_t idx, size_t M1, size_t N1,
				  template <typename, size_t> typename Container1,
                  typename ... Ts>
        constexpr void
        unpack(const Matrix<T, M1, N1, Container1>& first, Ts&& ... other) noexcept
        {
            const constexpr auto shift = indices(idx);
            const constexpr auto shift_i = shift.first;
            const constexpr auto shift_j = shift.second;
            const constexpr auto I = std::min(M - shift_i, M1);
            const constexpr auto J = std::min(N - shift_j, N1);
            for (size_t i = 0; i < I; ++i)
                for (size_t j = 0; j < J; ++j)
                    m_data[pos(shift_i + i, shift_j + j)] = first.at(i, j);
            unpack<pos(I, J, J) + 1>(std::forward<Ts>(other)...);
        }

    public:
		constexpr
    	Matrix() {}

		// single scalar construcor
    	explicit constexpr
    	Matrix(T&& rhs)
    	{ for (auto & elem : *this) elem = rhs;	}

    	explicit constexpr
    	Matrix(Data&& rhs) : m_data(rhs) {}

    	explicit constexpr
    	Matrix(Data& rhs) : m_data(rhs) {}

    	explicit constexpr
    	Matrix(const Data& rhs) : m_data(rhs) {}

		template <typename ... Ts>
		explicit constexpr
		Matrix(Ts&& ... rhs)
		{ unpack<0>(std::forward<Ts>(rhs)...); }

		constexpr Matrix&
		operator=(const Matrix& rhs) noexcept
		{
			m_data = rhs.m_data;
			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Matrix&
		operator=(const Matrix<T, M, N, Container2> & rhs) noexcept
		{
			std::copy(rhs.cbegin(), rhs.cend(), begin());
			return *this;
		}

		constexpr Matrix&
		operator=(const T & rhs) noexcept
		{
			for (T& i : m_data) i = rhs;
			return *this;
		}

		constexpr decltype(auto)
		operator[](const size_t i) const noexcept
		{
            CHECK_INDEX(i, M);
			return Vector(StaticSpan<T, N>(at(i,0)));
		}
		constexpr auto
		operator[](const size_t i) noexcept
		{
            CHECK_INDEX(i, M);
			return Vector(StaticSpan<T, N>(at(i,0)));
		}

		template <size_t I>
		constexpr auto
		get() const noexcept
		{ return Vector(StaticSpan<T, N>(std::get<pos(I, 0)>(m_data))); }

		template <size_t I>
		constexpr auto
		get() noexcept
		{ return Vector(StaticSpan<T, N>(std::get<pos(I, 0)>(m_data))); }

		const constexpr T&
		at(const size_t i) const noexcept
		{
            CHECK_INDEX(i, MN);
			return m_data[i];
		}
		constexpr T&
		at(const size_t i) noexcept
		{
            CHECK_INDEX(i, MN);
			return m_data[i];
		}

		const constexpr T&
		at(const size_t i, const size_t j) const noexcept
		{
            CHECK_INDEX(i, M);
            CHECK_INDEX(j, N);
			return m_data[pos(i, j)];
		}
		constexpr T&
		at(const size_t i, const size_t j) noexcept
		{
            CHECK_INDEX(i, M);
            CHECK_INDEX(j, N);
			return m_data[pos(i, j)];
		}

		constexpr auto
		begin() noexcept
		{ return m_data.begin(); }
		constexpr auto
		end() noexcept
		{ return m_data.end(); }
		constexpr decltype(auto)
		cbegin() const noexcept
		{ return m_data.cbegin(); }
		constexpr decltype(auto)
		cend() const noexcept
		{ return m_data.cend(); }

        constexpr decltype(auto)
        flat_ref() const noexcept
        { return Vector(StaticSpan<T, M * N>(at(0))); }

        constexpr decltype(auto)
        flat_ref() noexcept
        { return Vector(StaticSpan<T, M * N>(at(0))); }

		template <template <typename, size_t> typename Container2>
		constexpr Matrix
		operator+(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) + rhs.at(i);

			return ret;
		}
		constexpr Matrix
		operator+(const T & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) + rhs;

			return ret;
		}
		template <template <typename, size_t> typename Container2>
		constexpr Matrix&
		operator+=(const Matrix<T, M, N, Container2> & rhs)
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) += rhs.at(i);

			return *this;
		}
		constexpr Matrix&
		operator+=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) += rhs;

			return *this;
		}


		template <template <typename, size_t> typename Container2>
		constexpr Matrix
		operator-(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) - rhs.at(i);

			return ret;
		}
		constexpr Matrix
		operator-(const T & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) - rhs;

			return ret;
		}
		constexpr Matrix
		operator-() const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = -at(i);

			return ret;
		}
		template <template <typename, size_t> typename Container2>
		constexpr Matrix&
		operator-=(const Matrix<T, M, N, Container2> & rhs)
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) -= rhs.at(i);

			return *this;
		}
		constexpr Matrix&
		operator-=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) -= rhs;

			return *this;
		}

		// element-wise product
		template <template <typename, size_t> typename Container2>
		constexpr Matrix
		operator*(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) * rhs.at(i);

			return ret;
		}
		constexpr Matrix
		operator*(const T & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) * rhs;

			return ret;
		}
		template <template <typename, size_t> typename Container2>
		constexpr Matrix&
		operator*=(const Matrix<T, M, N, Container2> & rhs)
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) *= rhs.at(i);

			return *this;
		}
		constexpr Matrix&
		operator*=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) *= rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Matrix
		operator/(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) / rhs.at(i);

			return ret;
		}
		constexpr Matrix
		operator/(const T & rhs) const noexcept
		{
			Matrix ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) / rhs;

			return ret;
		}
		template <template <typename, size_t> typename Container2>
		constexpr Matrix&
		operator/=(const Matrix<T, M, N, Container2> & rhs)
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) /= rhs.at(i);

			return *this;
		}
		constexpr Matrix&
		operator/=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < MN; ++i)
				at(i) /= rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator==(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) == rhs.at(i);

			return ret;
		}
		constexpr auto
		operator==(const T & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) == rhs;

			return ret;
		}

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator!=(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) != rhs.at(i);

			return ret;
		}
		constexpr auto
		operator!=(const T & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) != rhs;

			return ret;
		}

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator<(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) < rhs.at(i);

			return ret;
		}
		constexpr auto
		operator<(const T & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) < rhs;

			return ret;
		}

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator<=(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) <= rhs.at(i);

			return ret;
		}
		constexpr auto
		operator<=(const T & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) <= rhs;

			return ret;
		}

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator>(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) > rhs.at(i);

			return ret;
		}
		constexpr auto
		operator>(const T & rhs) const noexcept
		{
			Matrix<bool, M, N> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) > rhs;

			return ret;
		}

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator>=(const Matrix<T, M, N, Container2> & rhs) const noexcept
		{
			Matrix<bool, M, N, Container> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) >= rhs.at(i);

			return ret;
		}
		constexpr auto
		operator>=(const T & rhs) const noexcept
		{
			Matrix<bool, M, N, Container> ret;

			for (size_t i = 0; i < MN; ++i)
				ret.at(i) = at(i) >= rhs;

			return ret;
		}

		template <typename Rhs>
		constexpr auto
		dot(const Rhs& rhs) const noexcept
		{ return lucid::dot(*this, rhs); }

		const constexpr Data&
		data() const noexcept
		{ return m_data; }

		static constexpr Matrix
		identity()
		{
			Matrix ret;

			for (size_t i = 0; i < M; ++i)
				for (size_t j = 0; j < N; ++j)
					ret.at(i, j) = i == j ? 1 : 0;

			return ret;
		}
    };

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
		std::array<size_t, M> idxs{};
		std::iota(idxs.begin(), idxs.end(), 0);

		auto product = [&idxs, &a]()
			{
				T ret{1};
				for (size_t i = 0; i < M; ++i)
					ret *= a.at(i, idxs[i]);
				return ret;
			};

		auto get_elem = [&]()
			{ return sgn(idxs) * product(); };

		T ret = get_elem();

		const constexpr size_t rank = fac(M) - 1;
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
				ret[i][j] = minus_one_pow(i+j) * det(minor_matrix(a, i, j));

		return ret;
    }

    // returns inverse matrix
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    inverse(const Matrix<T, M, N, Container> & a) noexcept
    {
        const auto is_zero = [](const T& val)
                             {
                                 if constexpr (std::is_floating_point_v<T>)
                                     return almost_equal(val, T{0}, 5);
                                 else
                                     return val == 0;
                             };

		const auto d = det(a);

		if (!is_zero(d))
			return transpose(cofactor(a)) / d;
		else
			return a;
    }
}

namespace std
{
	template <typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
	class tuple_size<lucid::Matrix<T, M, N, Container>>
    {
    public:
        static const constexpr size_t value = M;
    };

	template <size_t I, typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
	class tuple_element<I, lucid::Matrix<T, M, N, Container>>
	{
    public:
		using type = decltype(declval<lucid::Matrix<T, M, N, Container>>().template get<I>());
	};

    template <auto I, typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
    constexpr decltype(auto)
    get(const lucid::Matrix<T, M, N, Container>& mat) noexcept
    { return mat.template get<I>(); }
}
