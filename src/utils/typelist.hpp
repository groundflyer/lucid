// -*- C++ -*-
// utils.hpp
//

#pragma once

#include <tuple>

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
    struct typelist {};
}
