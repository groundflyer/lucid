// -*- C++ -*-
// intlist.hpp
//

/// @brief Defines list meta-funtions on std::integer_sequence

#pragma once

#include <utility>

namespace lucid
{
template <typename List1, typename List2>
struct join;

template <typename Int, Int... Ints1, Int... Ints2>
struct join<std::integer_sequence<Int, Ints1...>, std::integer_sequence<Int, Ints2...>>
{
    using type = std::integer_sequence<Int, Ints1..., Ints2...>;
};

template <std::size_t N, typename Int, Int... Vals>
struct repeat_integer;

template <typename Int, Int... Vals>
struct repeat_integer<1, Int, Vals...>
{
    using type = std::integer_sequence<Int, Vals...>;
};

template <std::size_t N, typename Int, Int... Vals>
struct repeat_integer
{
    using type = typename join<std::integer_sequence<Int, Vals...>,
                               typename repeat_integer<(N - 1), Int, Vals...>::type>::type;
};

template <std::size_t I, typename List>
struct elem;

template <typename Int, Int Head, Int... Vals>
struct elem<0, std::integer_sequence<Int, Head, Vals...>>
{
    static constexpr Int value = Head;
};

template <std::size_t I, typename Int, Int Head, Int... Vals>
struct elem<I, std::integer_sequence<Int, Head, Vals...>>
{
    static_assert(I < (sizeof...(Vals) + 1));

    static constexpr Int value = elem<I - 1, std::integer_sequence<Int, Vals...>>::value;
};

template <typename List>
struct reverse;

template <typename Int, Int x>
struct reverse<std::integer_sequence<Int, x>>
{
    using type = std::integer_sequence<Int, x>;
};

template <typename Int, Int x, Int... xs>
struct reverse<std::integer_sequence<Int, x, xs...>>
{
    using type = typename join<typename reverse<std::integer_sequence<Int, xs...>>::type,
                               std::integer_sequence<Int, x>>::type;
};

template <std::size_t N, typename ValType, ValType Val, typename List>
struct fill_val;

template <std::size_t N, typename Int, Int Val, Int... Vals>
struct fill_val<N, Int, Val, std::integer_sequence<Int, Vals...>>
{
    using type = typename join<std::integer_sequence<Int, Vals...>,
                               typename repeat_integer<N, Int, Val>::type>::type;
};
} // namespace lucid
