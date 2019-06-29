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
    class NormalN_: public ImmutableVectorOperators<T, N, Container, NormalN_>
    {
        static_assert(std::is_floating_point_v<T>, "Point consist of floating point elements.");

		using Data = Container<T, N>;

		Data m_data {};

    public:
		constexpr
		NormalN_() {}

        // don't call normalize when copying
        // this allows normals to have length != 1
        // in many cases it's convenient
		constexpr
		NormalN_(const NormalN_& rhs) : m_data(rhs.m_data) {}

		constexpr
		NormalN_(NormalN_&& rhs) : m_data(std::move(rhs.m_data)) {}

        template <template <typename, std::size_t> typename Container2>
        constexpr
        NormalN_(const NormalN_<T, 3, Container2>& rhs) : m_data(vector_constructor<0>(m_data, rhs)) {}

        template <template <typename, std::size_t> typename Container2>
        constexpr
        NormalN_(NormalN_<T, 3, Container2>&& rhs) : m_data(vector_constructor<0>(m_data, std::move(rhs))) {}

		explicit constexpr
		NormalN_(const Data& rhs) : m_data(rhs) { normalize(); }

		explicit constexpr
		NormalN_(Data&& rhs) : m_data(std::move(rhs)) { normalize(); }

        template <typename ... Ts>
        explicit constexpr
        NormalN_(Ts&& ... rhs) : m_data(vector_constructor<0>(m_data, std::forward<Ts>(rhs)...)) { normalize(); }

        NormalN_&
        operator=(const NormalN_& rhs) noexcept
        {
            m_data = rhs.m_data;
            return *this;
        }

        NormalN_&
        operator=(NormalN_&& rhs) noexcept
        {
            m_data = std::move(rhs.m_data);
            return *this;
        }

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
    NormalN_(Container<T, N> &&) -> NormalN_<T, N, Container>;
}

namespace std
{
	template <typename T, std::size_t N,
			  template <typename, std::size_t> typename Container>
	class tuple_size<lucid::NormalN_<T, N, Container>>
    {
    public:
        static const constexpr std::size_t value = N;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    class tuple_element<I, lucid::NormalN_<T, N, Container>>
	{
    public:
        using type = T;
    };

	template<std::size_t I, typename T, std::size_t N,
			 template <typename, std::size_t> typename Container>
    constexpr decltype(auto)
    get(const lucid::NormalN_<T, N, Container>& vec) noexcept
    { return vec.template get<I>(); }
}
