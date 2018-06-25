// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "types.hpp"

namespace yapt
{
    template <template <typename, size_t> typename Container>
    class Intersection_
    {
        bool m_intersect = false;
        Vector<real, 3, Container> m_data;

    public:
        constexpr
        Intersection_() {}

        constexpr
        Intersection_(const bool intersect, Container<real, 3>&& data)
        : m_intersect(intersect), m_data(data) {}

		template <template <typename, size_t> typename Container2>
		constexpr
		Intersection_(const bool intersect,
                      const real& t,
                      const Vec2_<Container2>& st) :
        m_intersect(intersect), m_data(t, st[0], st[1]) {}

        constexpr
        operator bool() const noexcept
        { return m_intersect; }

        constexpr decltype(auto)
        distance() const noexcept
        { return m_data[0]; }

        constexpr decltype(auto)
        distance() noexcept
        { return m_data[0]; }

        const constexpr auto
        coords() const noexcept
        { return Vector(ArrayView<real, 2>(const_cast<real*>(&m_data[1]))); }

        constexpr auto
        coords() noexcept
        { return Vector(ArrayView<real, 2>(&m_data[1])); }

        template <size_t I>
        decltype(auto)
        get() const noexcept
        {
            if constexpr (I == 0)
                return distance();
            else
                return coords();
        }

        template <size_t I>
        decltype(auto)
        get() noexcept
        {
            if constexpr (I == 0)
                return distance();
            else
                return coords();
        }
    };

	template <template <typename, size_t> typename Container>
	Intersection_(const bool, Container<real, 3>&&) -> Intersection_<Container>;

	template <template <typename, size_t> typename Container>
	Intersection_(const bool, const real&,
                  const Vec2_<Container>&) -> Intersection_<Container>;

    using Intersection = Intersection_<std::array>;
}

namespace std
{
    template <template <typename, size_t> typename Container>
    struct tuple_size<yapt::Intersection_<Container>> : integral_constant<size_t, 2> {};

    template <size_t I, template <typename, size_t> typename Container>
    struct tuple_element<I, yapt::Intersection_<Container>>
    {
        using type = decltype(declval<yapt::Intersection_<Container>>().template get<I>());
    };
}
