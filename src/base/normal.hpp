// -*- C++ -*-
// vector.hpp --
//

#pragma once

#include "vector_operators.hpp"

#include <utils/debug.hpp>


namespace lucid
{
    struct InvalidNormalException {};

    template <typename T, std::size_t N = 3,
			  template <typename, std::size_t> typename Container = std::array>
    class Normal_: public ImmutableVectorOperators<T, N, Container, Normal_>
    {
        static_assert(std::is_floating_point_v<T>, "Point consist of floating point elements.");

		using Data = Container<T, N>;

		Data m_data {};

    public:
		constexpr
		Normal_() {}

		constexpr
		Normal_(const Normal_& rhs) : m_data(rhs.m_data) { normalize(); }

		constexpr
		Normal_(Normal_&& rhs) : m_data(std::move(rhs.m_data)) { normalize(); }

		explicit constexpr
		Normal_(const Data& rhs) : m_data(rhs) { normalize(); }

		explicit constexpr
		Normal_(Data&& rhs) : m_data(std::move(rhs)) { normalize(); }

        template <typename ... Ts>
        explicit constexpr
        Normal_(Ts&& ... rhs) : m_data(vector_constructor<0>(m_data, std::forward<Ts>(rhs)...)) { normalize(); }

		constexpr decltype(auto)
		begin() const noexcept
		{ return m_data.begin(); }
		constexpr decltype(auto)
		end() const noexcept
		{ return m_data.end(); }
		constexpr decltype(auto)
		cbegin() const noexcept
		{ return m_data.cbegin(); }
		constexpr decltype(auto)
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
		constexpr const T&
		get() const noexcept
		{ return std::get<I>(m_data); }

		const constexpr Data&
		data() const noexcept
		{ return m_data; }

		constexpr auto
		size() const noexcept
		{ return N; }

		constexpr void
		normalize()
		{
			auto l = length(*this);

            if (almost_equal(l, T{1}, 5))
                return;

            if (almost_equal(l, T{0}, 5) ||
                any(transform(static_cast<bool(*)(T)>(std::isinf), *this)))
                throw InvalidNormalException{};

            for (auto& elem : m_data)
                elem /= l;
		}
    };

    template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
    Normal_(Container<T, N> &&) -> Normal_<T, N, Container>;
}

namespace std
{
	template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
	class tuple_size<lucid::Normal_<T, N, Container>>
    {
    public:
        static const constexpr std::size_t value = N;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    class tuple_element<I, lucid::Normal_<T, N, Container>>
	{
    public:
        using type = T;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    constexpr decltype(auto)
    get(const lucid::Normal_<T, N, Container>& vec) noexcept
    { return vec.template get<I>(); }
}
