// -*- C++ -*-
// intlist.hpp
//

/// @brief Defines list meta-funtions on std::integer_sequence

#pragma once

#include <utility>

namespace lucid
{
template <auto Val, typename List>
struct cons_int;

template <typename Int, Int Head, Int... Rest>
struct cons_int<Head, std::integer_sequence<Int, Rest...>>
{
    using type = std::integer_sequence<Int, Head, Rest...>;
};

template <auto Val, typename List>
using cons_int_t = typename cons_int<Val, List>::type;

template <typename List1, typename List2>
struct join;

template <typename Int, Int... Ints1, Int... Ints2>
struct join<std::integer_sequence<Int, Ints1...>, std::integer_sequence<Int, Ints2...>>
{
    using type = std::integer_sequence<Int, Ints1..., Ints2...>;
};

template <typename List1, typename List2>
using join_t = typename join<List1, List2>::type;

template <std::size_t N, typename Int, Int... Vals>
struct repeat_integers;

template <typename Int, Int... Vals>
struct repeat_integers<1, Int, Vals...>
{
    using type = std::integer_sequence<Int, Vals...>;
};

template <std::size_t N, typename Int, Int... Vals>
struct repeat_integers
{
    using type = join_t<std::integer_sequence<Int, Vals...>,
                               typename repeat_integers<(N - 1), Int, Vals...>::type>;
};

template <std::size_t N, typename Int, Int... Vals>
using repeat_integers_t = typename repeat_integers<N, Int, Vals...>::type;

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

template <std::size_t I, typename List>
using elem_v = typename elem<I, List>::value;

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
    using type = join_t<typename reverse<std::integer_sequence<Int, xs...>>::type,
                               std::integer_sequence<Int, x>>;
};

template <typename List>
using reverse_t = typename reverse<List>::type;

template <typename List, std::size_t N, auto Val>
struct fill_val;

template <std::size_t N, typename Int, Int Val, Int... Vals>
struct fill_val<std::integer_sequence<Int, Vals...>, N, Val>
{
    using type = join_t<std::integer_sequence<Int, Vals...>,
                               repeat_integers_t<N, Int, Val>>;
};

template <typename List, std::size_t N, auto Val>
using fill_val_t = typename fill_val<List, N, Val>::type;

template <typename List, auto Val>
struct del;

template <typename Int, Int Last, Int Val>
struct del<std::integer_sequence<Int, Last>, Val>
{
    using type = typename std::conditional_t<Last == Val, std::integer_sequence<Int>, std::integer_sequence<int, Last>>;
};

template <typename Int, Int Val, Int Head, Int... Rest>
struct del<std::integer_sequence<Int, Head, Rest...>, Val>
{
private:
    using rest = std::integer_sequence<Int, Rest...>;
public:
    using type = typename std::conditional_t<Head == Val, rest, cons_int_t<Head, typename del<rest, Val>::type>>;
};

template <typename List, auto Val>
using del_t = typename del<List, Val>::type;
} // namespace lucid
