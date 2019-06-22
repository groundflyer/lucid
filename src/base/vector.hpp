// -*- C++ -*-
// vector.hpp --
//

#pragma once

#include "vector_operators.hpp"

#include <utils/debug.hpp>


namespace lucid
{
    template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
    class Vector: public ImmutableVectorOperators<T, N, Container, Vector>,
        MutableVectorOperators<T, N, Container, Vector>
    {
    protected:
		using Data = Container<T, N>;

		Data m_data {};

        template <std::size_t idx>
        constexpr void
        unpack()
        {
            if constexpr (idx < N)
                for (auto i = idx; i < N; ++i)
                    m_data[i] = 0;
        }

        template <std::size_t idx, typename ... Ts>
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

		template <std::size_t idx, typename T1, std::size_t N1,
                  template <typename, std::size_t> typename Container1,
                  typename ... Ts>
        constexpr void
        unpack(const Vector<T1, N1, Container1>& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(std::size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + i] = static_cast<T>(first[i]);
            unpack<idx + N1>(std::forward<Ts>(other)...);
        }

        template <std::size_t idx, typename T1, std::size_t N1,
                  typename ... Ts>
        constexpr void
        unpack(const Container<T1, N1>& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(std::size_t i = 0; i < std::min(N - idx, N1); ++i)
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
		template <typename T2, std::size_t N2,
				  template <typename, std::size_t> typename Container2>
		constexpr
		Vector(const Vector<T2, N2, Container2> & rhs)
		{ for (std::size_t i = 0; i < std::min(N, N2); ++i) m_data[i] = static_cast<T>(rhs[i]); }

		constexpr Vector&
		operator=(const Vector & rhs) noexcept
		{
			m_data = rhs.m_data;
			return *this;
		}

		// between different containers we copy data manually
		template <template <typename, std::size_t> typename Container2>
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
		operator[](const std::size_t i) const noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}
		constexpr T&
		operator[](const std::size_t i) noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}

		template <std::size_t I>
		constexpr T&
		get() noexcept
		{ return std::get<I>(m_data); }

		template <std::size_t I>
		constexpr const T&
		get() const noexcept
		{ return std::get<I>(m_data); }

		const constexpr T&
		at(const std::size_t i) const noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}
		constexpr T&
		at(const std::size_t i) noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}

		const constexpr Data&
		data() const noexcept
		{ return m_data; }



		constexpr auto
		size() const noexcept
		{ return N; }
    };

    template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
    Vector(Container<T, N> &&) -> Vector<T, N, Container>;
}

namespace std
{
	template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
	class tuple_size<lucid::Vector<T, N, Container>>
    {
    public:
        static const constexpr std::size_t value = N;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    class tuple_element<I, lucid::Vector<T, N, Container>>
	{
    public:
        using type = T;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    constexpr decltype(auto)
    get(const lucid::Vector<T, N, Container>& vec) noexcept
    { return vec.template get<I>(); }
}
