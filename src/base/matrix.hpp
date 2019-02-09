// -*- C++ -*-
// matrix.hpp --
//

#pragma once


#include "matrix_ops.hpp"
#include "vector.hpp"


namespace yapt
{
    template <typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container = std::array>
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
		dot(Rhs && rhs) const noexcept
		{ return yapt::dot(*this, std::forward<Rhs>(rhs)); }

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
}

namespace std
{
	template <typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
	class tuple_size<yapt::Matrix<T, M, N, Container>>
    {
    public:
        static const constexpr size_t value = M;
    };

	template <size_t I, typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
	class tuple_element<I, yapt::Matrix<T, M, N, Container>>
	{
    public:
		using type = decltype(declval<yapt::Matrix<T, M, N, Container>>().template get<I>());
	};

    template <auto I, typename T, size_t M, size_t N,
			  template <typename, size_t> typename Container>
    constexpr decltype(auto)
    get(const yapt::Matrix<T, M, N, Container>& mat) noexcept
    { return mat.template get<I>(); }
}
