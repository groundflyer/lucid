// -*- C++ -*-
/// @file typelist.hpp
/// @brief Compile-time sequence of types and helper meta-functions.

#pragma once

#include <tuple>
#include <utility>
#include <variant>

namespace lucid
{
/// @brief Sequence of types.
template <typename... Ts>
struct type_sequence
{
};

namespace typelist
{
/// @brief Adds type @p T to the beginning of @p List.
template <typename T, typename List>
struct cons;

template <typename T, typename... Ts>
struct cons<T, type_sequence<Ts...>>
{
    using type = type_sequence<T, Ts...>;
};

template <typename T, typename List>
using cons_t = typename cons<T, List>::type;

/// @brief Length of an std::integer_sequence.
template <typename List>
struct length;

template <typename... Ts>
struct length<type_sequence<Ts...>>
{
    static constexpr std::size_t value = sizeof...(Ts);
};

template <typename List>
constexpr std::size_t length_v = length<List>::value;

/// @brief Appends two lists together.
template <typename A, typename B>
struct join;

template <typename... Ts1, typename... Ts2>
struct join<type_sequence<Ts1...>, type_sequence<Ts2...>>
{
    using type = type_sequence<Ts1..., Ts2...>;
};

template <typename A, typename B>
using join_t = typename join<A, B>::type;

/// @brief Constructs type_sequence repeating @p Ts @p N times.
template <std::size_t N, typename... Ts>
struct repeat;

template <typename... Ts>
struct repeat<1, Ts...>
{
    using type = type_sequence<Ts...>;
};

template <std::size_t N, typename... Ts>
struct repeat
{
    using type = join_t<type_sequence<Ts...>, typename repeat<(N - 1), Ts...>::type>;
};

template <std::size_t N, typename... Ts>
using repeat_t = typename repeat<N, Ts...>::type;

/// @brief Use types from @p List to build new type from variadic template @p Container.
template <template <typename...> typename Container, typename List>
struct repack;

template <template <typename...> typename Container, typename... Ts>
struct repack<Container, type_sequence<Ts...>>
{
    using type = Container<Ts...>;
};

template <template <typename...> typename Container, typename List>
using repack_t = typename repack<Container, List>::type;

template <typename List>
using as_tuple = repack_t<std::tuple, List>;

template <typename List>
using as_variant = repack_t<std::variant, List>;

/// @brief Apply @p Func to every element of the @p List.
template <template <typename> typename Func, typename List>
struct map;

template <template <typename> typename Func, typename... Ts>
struct map<Func, type_sequence<Ts...>>
{
    using type = type_sequence<Func<Ts>...>;
};

template <template <typename> typename Func, typename List>
using map_t = typename map<Func, List>::type;

/// @brief Extracts the @p Ith type from the @p List.
template <std::size_t I, typename List>
using elem_t = std::tuple_element_t<I, as_tuple<List>>;

/// @brief Extracts the first type from the @p List.
template <typename List>
using head = elem_t<0, List>;

/// @brief Extracts the last type from the @p List.
template <typename List>
using last = elem_t<length_v<List> - 1, List>;

template <typename T, std::size_t Idx, typename List>
struct find;

template <typename T, std::size_t Idx, typename Last>
struct find<T, Idx, type_sequence<Last>>
{
    static_assert(!std::is_same_v<T, Last>, "Type not found");
    static constexpr std::size_t value = Idx;
};

template <typename T, std::size_t Idx, typename Head, typename... Rest>
struct find<T, Idx, type_sequence<Head, Rest...>>
{
    static constexpr std::size_t value =
        std::is_same_v<T, Head> ? Idx : find<T, Idx + 1, type_sequence<Rest...>>::value;
};

/// @brief Get the index of type @p T in @p List.
template <typename T, typename List>
constexpr std::size_t find_v = find<T, 0, List>::value;

/// @brief Build type_sequence containing first @p N types from @p List.
template <std::size_t N, typename List>
struct take;

template <typename Head, typename... Rest>
struct take<1, type_sequence<Head, Rest...>>
{
    using type = type_sequence<Head>;
};

template <std::size_t N, typename Head, typename... Rest>
struct take<N, type_sequence<Head, Rest...>>
{
    static_assert(sizeof...(Rest) >= N && N > 0);

    using type = cons_t<Head, typename take<N - 1, type_sequence<Rest...>>::type>;
};

template <std::size_t N, typename List>
using take_t = typename take<N, List>::type;

/// @brief Build std::index_sequence for @p List.
template <typename List>
using indicies = std::make_index_sequence<length_v<List>>;

/// @brief Build a type_sequence of result types of @p Funcs applied to @p Args.
template <typename Funcs, typename... Args>
struct funcs_result
{
    template <typename F>
    using f = std::invoke_result_t<F, Args...>;

    using type = map_t<f, Funcs>;
};

template <typename Funcs, typename... Args>
using funcs_result_t = typename funcs_result<Funcs, Args...>::type;

/// @brief Deduce result type of @p Func applied to arguments from @p Args typelist.
template <typename Args, typename Func>
struct args_result;

template <typename Func, typename... Args>
struct args_result<type_sequence<Args...>, Func>
{
    using type = std::invoke_result_t<Func, Args...>;
};

template <typename Args, typename Func>
using args_result_t = typename args_result<Args, Func>::type;

/// @brief Checks if @p List consist of same elements.
template <typename List>
struct same_types;

template <typename Head, typename... Ts>
struct same_types<type_sequence<Head, Ts...>>
{
    static constexpr bool value = (true && ... && std::is_same_v<Head, Ts>);
};

template <typename List>
constexpr bool same_types_v = same_types<List>::value;
} // namespace typelist
} // namespace lucid
