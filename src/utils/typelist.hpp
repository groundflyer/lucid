// -*- C++ -*-
// utils.hpp
//

#pragma once

#include <tuple>
#include <variant>

namespace lucid
{
    template <size_t I, typename... Ts>
    using pack_element = typename std::tuple_element<I, std::tuple<Ts...>>::type;

    template <size_t I, typename Typelist>
    struct typelist_element;

    template <size_t I, template <typename...> typename Typelist, typename... Ts>
    struct typelist_element<I, Typelist<Ts...>>
    {
        using type = pack_element<I, Ts...>;
    };

    template <typename... Ts>
    struct typelist
    {
        using variant = typename std::variant<Ts...>;
        using tuple = typename std::tuple<Ts...>;

        template <template <typename> typename Array>
        using tuple_of_arrays = std::tuple<Array<Ts>...>;
    };
}
