// -*- C++ -*-
// vec.hpp --
//

#pragma once

#include "debug.hpp"
#include "vec_ops.hpp"

#include <iostream>		// operator <<
#include <type_traits>
#include <initializer_list>


namespace yapt
{
    template <typename T, size_t N,
			  template <typename, size_t> typename Container>
    class Vector
    {
		static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

    protected:
		using Data = Container<T, N>;

		Data m_data {};

    public:
		constexpr
		Vector() {}

		constexpr
		Vector(const Vector & rhs) : m_data(rhs.m_data) {}

		explicit constexpr
		Vector(const Data & rhs) : m_data(rhs) {}

		explicit constexpr
		Vector(const T & rhs) { for (T& i : m_data) i = rhs; }

		constexpr
		Vector(Vector && rhs) : m_data(std::forward<Data>(rhs.m_data)) {}

		// conversion constructor
		template <typename T2, size_t N2,
				  template <typename, size_t> typename Container2>
		constexpr
		Vector(const Vector<T2, N2, Container2> & rhs)
		{ for (size_t i = 0; i < std::min(N, N2); ++i) m_data[i] = static_cast<T>(rhs[i]); }

		explicit constexpr
		Vector(Data && rhs) : m_data(std::forward<Data>(rhs)) {}

		explicit constexpr
		Vector(T && rhs) { for (T& i : m_data) i = rhs; }

		explicit constexpr
		Vector(std::initializer_list<T> l)
		{ std::copy(l.begin(), l.begin()+std::min(l.size(), N), begin()); }

		template <template <typename, size_t> typename Container2>
		explicit constexpr
		Vector(const Container2<T, N> & rhs)
		{ std::copy(rhs.cbegin(), rhs.cend(), begin());	}

		template <class ... Types>
		explicit constexpr
		Vector(const T & first,
			   const Types & ... args) : m_data({{first, args...}})
		{
			static_assert(sizeof...(args) <= N - 1,
						  "The number of elements doesn't match!");
		}

		template <class ... Types>
		explicit constexpr
		Vector(T && first,
			   Types && ... args) :
		m_data({{std::forward<T>(first), (std::forward<T>(args))...}})
		{
			static_assert(sizeof...(args) <= N - 1,
						  "The number of elements doesn't match!");
		}

		constexpr Vector&
		operator=(const Vector & rhs) noexcept
		{
			m_data = rhs.m_data;
			return *this;
		}

		// between different containers we copy data manually
		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator=(const Vector<T, N, Container2> & rhs) noexcept
		{
			std::copy(rhs.cbegin(), rhs.cend(), begin());
			return *this;
		}

		constexpr Vector&
		operator=(const T & rhs) noexcept
		{
			for (T& i : m_data) i = rhs;
			return *this;
		}

		constexpr auto
		begin() noexcept
		{ return m_data.begin(); }
		constexpr auto
		end() noexcept
		{ return m_data.end(); }
		constexpr auto
		cbegin() const noexcept
		{ return m_data.cbegin(); }
		constexpr auto
		cend() const noexcept
		{ return m_data.cend(); }

		const constexpr T&
		operator[](const size_t i) const noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}
		constexpr T&
		operator[](const size_t i) noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}

		template <size_t I>
		constexpr T&
		get() noexcept
		{ return m_data[I]; }

		template <size_t I>
		constexpr const T&
		get() const noexcept
		{ return m_data[I]; }

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

		const constexpr Data&
		data() const noexcept
		{ return m_data; }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator+(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::plus<T>()); }
		constexpr auto
		operator+(const T & rhs) const noexcept
		{ return transform(*this, [&rhs](const T& a){ return a + rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator-(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::minus<T>()); }
		constexpr auto
		operator-(const T & rhs) const noexcept
		{ return transform(*this, [&rhs](const T& a){ return a - rhs; }); }
		constexpr auto
		operator-() const noexcept
		{ return transform(*this, std::negate<T>()); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator*(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::multiplies<T>()); }
		constexpr auto
		operator*(const T & rhs) const noexcept
		{ return transform(*this, [&rhs](const T& a){ return a * rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator/(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::divides<T>()); }
		constexpr auto
		operator/(const T & rhs) const noexcept
		{ return transform(*this, [&rhs](const T& a){ return a / rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator+=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] += rhs[i];

			return *this;
		}
		constexpr Vector&
		operator+=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] += rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator-=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] -= rhs[i];

			return *this;
		}
		constexpr Vector&
		operator-=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] -= rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator*=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] *= rhs[i];

			return *this;
		}
		constexpr Vector&
		operator*=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] *= rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator/=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] /= rhs[i];

			return *this;
		}

		constexpr Vector&
		operator/=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] /= rhs;

			return *this;
		}


		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator==(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::equal_to<T>()); }
		constexpr auto
		operator==(const T& rhs) const noexcept
		{ return transform(*this, [&rhs](const T& elem){ return elem == rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator!=(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::not_equal_to<T>()); }
		constexpr auto
		operator!=(const T& rhs) const noexcept
		{ return transform(*this, [&rhs](const T& elem){ return elem != rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator>(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::greater<T>()); }
		constexpr auto
		operator>(const T& rhs) const noexcept
		{ return transform(*this, [&rhs](const T& elem){ return elem > rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator<(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::less<T>()); }
		constexpr auto
		operator<(const T& rhs) const noexcept
		{ return transform(*this, [&rhs](const T& elem){ return elem < rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator>=(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::greater_equal<T>()); }
		constexpr auto
		operator>=(const T& rhs) const noexcept
		{ return transform(*this, [&rhs](const T& elem){ return elem >= rhs; }); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator<=(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(*this, rhs, std::less_equal<T>()); }
		constexpr auto
		operator<=(const T& rhs) const noexcept
		{ return transform(*this, [&rhs](const T& elem){ return elem <= rhs; }); }

        constexpr auto
        operator!() const noexcept
        { return transform(*this, std::logical_not<T>()); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		dot(const Vector<T, N, Container2> & rhs) const noexcept
		{ return yapt::dot(*this, rhs); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		cross(const Vector<T, N, Container2> & rhs) const noexcept
		{ return yapt::cross(*this, rhs); }

		constexpr auto
		length() const noexcept
		{ return yapt::length(*this); }

		constexpr auto
		size() const noexcept
		{ return N; }

		friend std::ostream&
		operator<<(std::ostream & os, const Vector & rhs) noexcept
		{
            for (size_t i = 0; i < N - 1; ++i)
                os << rhs.m_data[i] << ' ';
            os << rhs.m_data[N-1];

			return os;
		}
    };

    template <typename T, size_t N,
			  template <typename, size_t> typename Container>
    Vector(Container<T, N> &&) -> Vector<T, N, Container>;
}

namespace std
{
	template <typename T, size_t N,
			  template <typename, size_t> typename Container>
	struct tuple_size<yapt::Vector<T, N, Container>> : integral_constant<size_t, N> {};

	template<size_t I, typename T, size_t N,
			 template <typename, size_t> typename Container>
    struct tuple_element<I, yapt::Vector<T, N, Container>>
	{ using type = T; };
}
