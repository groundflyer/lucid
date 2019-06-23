// -*- C++ -*-
// vector.hpp --
//

#pragma once

#include "vector_operators.hpp"

#include <utils/debug.hpp>


namespace lucid
{
    struct InvalidPointException {};

    template <typename T, std::size_t N = 3,
			  template <typename, std::size_t> typename Container = std::array>
    class Point_:
        public ImmutableVectorOperators<T, N, Container, Point_>,
        public MutableVectorOperators<T, N, Container, Point_>
    {
        static_assert(std::is_floating_point_v<T>, "Point consist of floating point elements.");

		using Data = Container<T, N>;
		Data m_data {};

        constexpr void
        check() const
        {
            if (any(transform(static_cast<bool(*)(T)>(std::isinf), *this)))
                throw InvalidPointException{};
        }

    public:
		constexpr
		Point_() {}

		constexpr
		Point_(const Point_& rhs) : m_data(rhs.m_data) { check(); }

		constexpr
		Point_(Point_&& rhs) : m_data(std::move(rhs.m_data)) { check(); }

		explicit constexpr
		Point_(const Data& rhs) : m_data(rhs) { check(); }

		explicit constexpr
		Point_(Data&& rhs) : m_data(std::move(rhs)) { check(); }

        template <typename ... Ts>
        explicit constexpr
        Point_(Ts&& ... rhs) : m_data(vector_constructor<0>(m_data, std::forward<Ts>(rhs)...))
        { check(); }

		constexpr Point_&
		operator=(const Point_& rhs) noexcept
		{
			m_data = rhs.m_data;
            check();
			return *this;
		}

		constexpr Point_&
		operator=(Point_&& rhs) noexcept
		{
			m_data = std::move(rhs.m_data);
            check();
			return *this;
		}


		constexpr decltype(auto)
		operator=(const T& rhs) noexcept
		{
            for (std::size_t i = 0; i < N; ++i)
                m_data[i] = rhs;
            check();
			return *this;
		}

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr decltype(auto)
		operator=(const VectorType2<T, N, Container2>& rhs) noexcept
        {
            for (std::size_t i = 0; i < N; ++i)
                m_data[i] = rhs[i];
            check();
			return *this;
        }

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

		constexpr decltype(auto)
		begin() const noexcept
		{ return m_data.begin(); }
		constexpr decltype(auto)
		end() const noexcept
		{ return m_data.end(); }
		constexpr decltype(auto)
		begin() noexcept
		{ return m_data.begin(); }
		constexpr decltype(auto)
		end() noexcept
		{ return m_data.end(); }
		constexpr decltype(auto)
		cbegin() const noexcept
		{ return m_data.cbegin(); }
		constexpr decltype(auto)
		cend() const noexcept
		{ return m_data.cend(); }

		template <std::size_t I>
		constexpr const T&
		get() const noexcept
		{ return std::get<I>(m_data); }

		template <std::size_t I>
		constexpr T&
		get() noexcept
		{ return std::get<I>(m_data); }

		const constexpr Data&
		data() const noexcept
		{ return m_data; }
		const constexpr Data&
		data() noexcept
		{ return m_data; }

		constexpr auto
		size() const noexcept
		{ return N; }
    };

    template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
    Point_(Container<T, N> &&) -> Point_<T, N, Container>;
}

namespace std
{
	template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
	class tuple_size<lucid::Point_<T, N, Container>>
    {
    public:
        static const constexpr std::size_t value = N;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    class tuple_element<I, lucid::Point_<T, N, Container>>
	{
    public:
        using type = T;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    constexpr decltype(auto)
    get(const lucid::Point_<T, N, Container>& vec) noexcept
    { return vec.template get<I>(); }
}
