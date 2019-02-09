// -*- C++ -*-
// vector.hpp --
//

#pragma once

// we don't use StaticSpan her, but if don't include it
// a compiler would be unable to find std::get specialization
// for Vector's Container
#include "static_span.hpp"

#include "debug.hpp"
#include "vector_ops.hpp"

#include <array>
#include <type_traits>


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

        template <size_t idx>
        constexpr void
        unpack()
        {
            if constexpr (idx < N)
                for (auto i = idx; i < N; ++i)
                    m_data[i] = 0;
        }

        template <size_t idx, typename ... Ts>
        constexpr void
        unpack(const T& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            m_data[idx] = first;
            if constexpr (sizeof...(other) == 0 && idx < N-1)
                unpack<idx+1>(first);
            else
                unpack<idx+1>(std::forward<Ts>(other)...);
        }

		template <size_t idx, typename T1, size_t N1,
                  template <typename, size_t> typename Container1,
                  typename ... Ts>
        constexpr void
        unpack(const Vector<T1, N1, Container1>& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + i] = static_cast<T>(first[i]);
            unpack<idx + N1>(std::forward<Ts>(other)...);
        }

        template <size_t idx, typename T1, size_t N1,
                  typename ... Ts>
        constexpr void
        unpack(const Container<T1, N1>& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + i] = static_cast<T>(first[i]);
            unpack<idx + N1>(std::forward<Ts>(other)...);
        }

    public:
		constexpr
		Vector() {}

		constexpr
		Vector(const Vector& rhs) : m_data(rhs.m_data) {}

		explicit constexpr
		Vector(Data && rhs) : m_data(std::forward<Data>(rhs)) {}

		explicit constexpr
		Vector(const Data& rhs) : m_data(rhs) {}

		explicit constexpr
		Vector(Data& rhs) : m_data(rhs) {}

		explicit constexpr
		Vector(T && rhs) { for (auto& i : m_data) i = rhs; }

        template <typename ... Ts>
        explicit constexpr
        Vector(Ts && ... rhs)
        { unpack<0>(std::forward<Ts>(rhs)...); }

		// // conversion constructor
		template <typename T2, size_t N2,
				  template <typename, size_t> typename Container2>
		constexpr
		Vector(const Vector<T2, N2, Container2> & rhs)
		{ for (size_t i = 0; i < std::min(N, N2); ++i) m_data[i] = static_cast<T>(rhs[i]); }

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
		{ return std::get<I>(m_data); }

		template <size_t I>
		constexpr const T&
		get() const noexcept
		{ return std::get<I>(m_data); }

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
    };

    template <typename T, size_t N,
			  template <typename, size_t> typename Container>
    Vector(Container<T, N> &&) -> Vector<T, N, Container>;
}

namespace std
{
	template <typename T, size_t N,
			  template <typename, size_t> typename Container>
	class tuple_size<yapt::Vector<T, N, Container>>
    {
    public:
        static const constexpr size_t value = N;
    };

	template<size_t I, typename T, size_t N,
			 template <typename, size_t> typename Container>
    class tuple_element<I, yapt::Vector<T, N, Container>>
	{
    public:
        using type = T;
    };

	template<size_t I, typename T, size_t N,
			 template <typename, size_t> typename Container>
    constexpr decltype(auto)
    get(const yapt::Vector<T, N, Container>& vec) noexcept
    { return vec.template get<I>(); }
}
