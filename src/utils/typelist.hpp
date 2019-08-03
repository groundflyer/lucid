// -*- C++ -*-
// utils.hpp
//

#pragma once

#include <tuple>
#include <variant>

namespace lucid
{
    template <typename Head, typename... Ts>
    struct typelist
    {
        using head = Head;
        using variant = typename std::variant<head, Ts...>;
        using tuple = typename std::tuple<head, Ts...>;

        template <template <typename> typename Array>
        using tuple_of_arrays = std::tuple<Array<head>, Array<Ts>...>;

        template <std::size_t I>
        using get = std::tuple_element_t<I, tuple>;

        static const constexpr std::size_t size = sizeof...(Ts) + 1;
        static const constexpr bool same = (true && ... && std::is_same_v<head, Ts>);

        using indicies = std::make_index_sequence<size>;

        constexpr
        typelist(std::tuple<Ts...>) noexcept {}

        constexpr
        typelist(std::variant<Ts...>) noexcept {}
    };
}
