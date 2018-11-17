// -*- C++ -*-
// matrix.hpp --
//

#pragma once


#include "vector.hpp"
#include "matrix_ops.hpp"
#include "arrayview.hpp"


namespace yapt
{
    template <typename T, size_t R, size_t C,
			  template <typename, size_t> typename Container = std::array>
    class Matrix
    {
		static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");
		static_assert(R > 1 || C > 1, "One-dimensional matrices are not supported.");

		static const constexpr size_t N = R * C;

		using Data = Container<T, N>;
		Data m_data {};

		static constexpr size_t
		pos(const size_t i, const size_t j, const size_t J = C)
		{ return i * J + j; }

		// variyng template sink
        template <size_t>
		constexpr void
        unpack() const {}

        template <size_t idx, typename ... Ts>
        constexpr void
        unpack(const T& first, Ts && ... other)
        {
            static_assert(idx < N, "Too many elements.");
            m_data[idx] = first;
            if constexpr (sizeof...(other) == 0 && idx < N-1)
                unpack<idx+1>(first);
            else
                unpack<idx+1>(std::forward<Ts>(other)...);
        }

		// unpack vector arguments
		template <size_t idx,
                  typename T1, size_t N1,
                  template <typename, size_t> typename Container2,
				  typename ... Ts>
		constexpr void
        unpack(const Vector<T1, N1, Container2>& first,
               Ts&& ... other)
		{
            static_assert(idx < N, "Too many elements.");
            for(size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + 1] = static_cast<T>(first[i]);
			unpack<idx + N1>(std::forward<Ts>(other)...);
		}

        template <size_t idx, typename T1, size_t N1,
                  typename ... Ts>
        constexpr void
        unpack(const Container<T1, N1>& first, Ts && ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + i] = static_cast<T>(first[i]);
            unpack<idx + N1>(std::forward<Ts>(other)...);
        }

		template <size_t idx, typename T1, size_t R1, size_t C1,
				  template <typename, size_t> typename Container1,
                  typename ... Ts>
        constexpr void
        unpack(const Matrix<T1, R1, C1, Container1>& first, Ts&& ... other)
        {
            const constexpr auto I = std::min(R, R1);
            const constexpr auto J = std::min(C, C1);
            for (size_t i = idx % R; i < I; ++i)
                for (size_t j = idx / C; j < J; ++j)
                    m_data[pos(i, j)] = static_cast<T>(first[i][j]);
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
		operator=(const Matrix<T, R, C, Container2> & rhs) noexcept
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

		const constexpr auto
		operator[](const size_t i) const noexcept
		{
            CHECK_INDEX(i, R);
			return Vector(ArrayView<T, C>(const_cast<T*>(&(at(i,0)))));
		}
		constexpr auto
		operator[](const size_t i) noexcept
		{
            CHECK_INDEX(i, R);
			return Vector(ArrayView<T, C>(&(at(i,0))));
		}

		template <size_t I>
		constexpr decltype(auto)
		get() const noexcept
		{ return (*this)[I]; }

		template <size_t I>
		constexpr decltype(auto)
		get() noexcept
		{ return (*this)[I]; }

		const constexpr T&
		at(const size_t i) const noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}
		constexpr T&
		at(const size_t i) noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}

		const constexpr T&
		at(const size_t i, const size_t j) const noexcept
		{
            CHECK_INDEX(i, R);
            CHECK_INDEX(j, C);
			return m_data[pos(i, j)];
		}
		constexpr T&
		at(const size_t i, const size_t j) noexcept
		{
            CHECK_INDEX(i, R);
            CHECK_INDEX(j, C);
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

		template <template <typename, size_t> typename Container2>
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
		template <template <typename, size_t> typename Container2>
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


		template <template <typename, size_t> typename Container2>
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
		template <template <typename, size_t> typename Container2>
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
		template <template <typename, size_t> typename Container2>
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
		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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
		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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

		template <template <typename, size_t> typename Container2>
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
		{ return m_data; }

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
            for (size_t i = 0; i < N - 1; ++i)
                os << rhs.m_data[i] << ' ';
            os << rhs.m_data[N-1];

    	    return os;
    	}
    };
}

namespace std
{
	template <typename T, size_t R, size_t C,
			  template <typename, size_t> typename Container>
	struct tuple_size<yapt::Matrix<T, R, C, Container>> : integral_constant<size_t, R> {};

	template <size_t I, typename T, size_t R, size_t C,
			  template <typename, size_t> typename Container>
	struct tuple_element<I, yapt::Matrix<T, R, C, Container>>
	{
		using type = decltype(declval<yapt::Matrix<T, R, C, Container>>().template get<I>());
	};
}
