// -*- C++ -*-
/// @file intlist.hpp
/// @brief List operations on std::integer_sequence.

#pragma once

#include "math.hpp"

#include <utility>

namespace lucid::intlist
{
/// @brief Adds element @p Val to the beginning of @p List.
template <auto Val, typename List>
struct cons;

template <typename Int, Int Head>
struct cons<Head, std::integer_sequence<Int>>
{
    using type = std::integer_sequence<Int, Head>;
};

template <typename Int, Int Head, Int... Rest>
struct cons<Head, std::integer_sequence<Int, Rest...>>
{
    using type = std::integer_sequence<Int, Head, Rest...>;
};

template <auto Val, typename List>
using cons_t = typename cons<Val, List>::type;

/// @brief Length of an std::integer_sequence.
template <typename List>
struct length;

template <typename Int, Int... Nums>
struct length<std::integer_sequence<Int, Nums...>>
{
    static constexpr std::size_t value = sizeof...(Nums);
};

template <typename List>
static constexpr std::size_t length_v = length<List>::value;

/// @brief Appends two lists together.
template <typename List1, typename List2>
struct join;

template <typename Int, Int... Ints1, Int... Ints2>
struct join<std::integer_sequence<Int, Ints1...>, std::integer_sequence<Int, Ints2...>>
{
    using type = std::integer_sequence<Int, Ints1..., Ints2...>;
};

template <typename List1, typename List2>
using join_t = typename join<List1, List2>::type;

/// @brief Constructs std::integer_sequence repeating @p Vals @p N times.
template <std::size_t N, typename Int, Int... Vals>
struct repeat;

template <typename Int, Int... Vals>
struct repeat<0, Int, Vals...>
{
    using type = std::integer_sequence<Int>;
};

template <std::size_t N, typename Int, Int... Vals>
struct repeat
{
    using type =
        join_t<std::integer_sequence<Int, Vals...>, typename repeat<N - 1, Int, Vals...>::type>;
};

template <std::size_t N, typename Int, Int... Vals>
using repeat_t = typename repeat<N, Int, Vals...>::type;

/// @brief Extracts the @p Ith elementh from the @p List.
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
static constexpr auto elem_v = elem<I, List>::value;

/// @brief Reverses the order of the elements in @p List.
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

/// @brief Appends @p Val @p N times to the end of @p List.
template <typename List, std::size_t N, auto Val>
struct nappend;

template <std::size_t N, typename Int, Int Val, Int... Vals>
struct nappend<std::integer_sequence<Int, Vals...>, N, Val>
{
    using type = join_t<std::integer_sequence<Int, Vals...>, repeat_t<N, Int, Val>>;
};

template <typename List, std::size_t N, auto Val>
using nappend_t = typename nappend<List, N, Val>::type;

template <typename List, auto Val>
using append_t = nappend_t<List, 1, Val>;

/// @brief Deletes @p Val from @p List.
template <typename List, auto Val>
struct del;

template <typename Int, Int Last, Int Val>
struct del<std::integer_sequence<Int, Last>, Val>
{
    using type = std::
        conditional_t<Last == Val, std::integer_sequence<Int>, std::integer_sequence<Int, Last>>;
};

template <typename Int, Int Val, Int Head, Int... Rest>
struct del<std::integer_sequence<Int, Head, Rest...>, Val>
{
  private:
    using rest = std::integer_sequence<Int, Rest...>;

  public:
    using type = std::conditional_t<Head == Val, rest, cons_t<Head, typename del<rest, Val>::type>>;
};

template <typename List, auto Val>
using del_t = typename del<List, Val>::type;

/// @brief Factorial representation of number @p N.
///
/// See https://en.wikipedia.org/wiki/Factorial_number_system
/// and https://en.wikipedia.org/wiki/Lehmer_code
template <std::size_t N, std::size_t I>
struct lehmer_code
{
  private:
    static constexpr std::size_t quot = N / I;
    static constexpr std::size_t rem  = N % I;

  public:
    using type = append_t<typename lehmer_code<quot, I + 1>::type, rem>;
};

template <std::size_t I>
struct lehmer_code<0, I>
{
    using type = std::index_sequence<>;
};

template <std::size_t N>
using lehmer_code_t = typename lehmer_code<N, 1>::type;

/// @brief Generates a permutation of index array @p Idxs from
/// its factorial representation @p FacNum.
template <typename Idxs, typename FacNum>
struct lehmer_perm;

template <typename Idxs>
struct lehmer_perm<Idxs, std::index_sequence<>>
{
    using type = std::index_sequence<>;
};

template <typename Idxs, std::size_t Head, std::size_t... Rest>
struct lehmer_perm<Idxs, std::index_sequence<Head, Rest...>>
{
  private:
    static constexpr std::size_t _elem = elem_v<Head, Idxs>;

  public:
    using type =
        cons_t<_elem, typename lehmer_perm<del_t<Idxs, _elem>, std::index_sequence<Rest...>>::type>;
};

template <typename Idxs, typename FacNum>
using lehmer_perm_t = typename lehmer_perm<Idxs, FacNum>::type;

/// @brief @p Nth permutation in lexicographical order
/// of an index sequence of length @p L.
template <std::size_t N, std::size_t L>
struct nlperm
{
    static_assert(N < fac(L), "N should be less than L!");

  private:
    using _lehmer_code                = lehmer_code_t<N>;
    static constexpr std::size_t diff = L - length_v<_lehmer_code>;

  public:
    using type = lehmer_perm_t<std::make_index_sequence<L>,
                               join_t<repeat_t<diff, std::size_t, 0ul>, _lehmer_code>>;
};

template <std::size_t N, std::size_t L>
using nlperm_t = typename nlperm<N, L>::type;

/// @brief Converts std::integer_sequence into std::array.
template <typename Int, Int... Is>
constexpr std::array<Int, sizeof...(Is)> to_array(std::integer_sequence<Int, Is...>) noexcept
{
    return std::array<Int, sizeof...(Is)>{Is...};
}
} // namespace lucid::intlist
