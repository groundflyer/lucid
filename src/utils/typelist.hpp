// -*- C++ -*-
// utils.hpp
//

#pragma once

#include <tuple>
#include <utility>
#include <variant>

namespace lucid
{
namespace detail
{
template <std::size_t idx, typename Sought, typename Head, typename... Rest>
constexpr std::size_t
index_impl() noexcept
{
    if constexpr(std::is_same_v<Sought, Head>)
        return idx;
    else if constexpr(sizeof...(Rest) > 0ul)
        return index_impl<idx + 1, Sought, Rest...>();
    else
        static_assert(sizeof...(Rest) > 0ul, "There is no such type in the typelist.");
}
} // namespace detail

template <typename... Ts>
struct typelist
{
    template <template <typename...> typename Container>
    using repack = Container<Ts...>;

    template <template <typename> typename Func, template <typename...> typename Container>
    using map = Container<Func<Ts>...>;

    using variant = repack<std::variant>;
    using tuple   = repack<std::tuple>;

    template <std::size_t I>
    using at = std::tuple_element_t<I, tuple>;

    template <typename Type>
    static constexpr std::size_t
    index() noexcept
    {
        return detail::index_impl<0, Type, Ts...>();
    }

    static const constexpr std::size_t size = sizeof...(Ts);

    using front = at<0>;
    using back  = at<size - 1>;

    static const constexpr bool        same = (true && ... && std::is_same_v<front, Ts>);

    using indices = std::make_index_sequence<size>;

    template <typename... Args>
    using result_of = typelist<std::invoke_result_t<Ts, Args...>...>;

    constexpr explicit typelist(std::tuple<Ts...>) noexcept {}

    constexpr explicit typelist(std::variant<Ts...>) noexcept {}

    constexpr typelist() noexcept {}
};

template <typename Seq1, typename Seq2>
struct join;

template <typename... Ts1, typename... Ts2>
struct join<typelist<Ts1...>, typelist<Ts2...>>
{
    using type = typelist<Ts1..., Ts2...>;
};

template <typename Int, Int... Ints1, Int... Ints2>
struct join<std::integer_sequence<Int, Ints1...>, std::integer_sequence<Int, Ints2...>>
{
    using type = std::integer_sequence<Int, Ints1..., Ints2...>;
};

template <std::size_t N, typename... Ts>
struct repeat_type;

template <typename... Ts>
struct repeat_type<1, Ts...>
{
    using type = typelist<Ts...>;
};

template <std::size_t N, typename... Ts>
struct repeat_type
{
    using type = typename join<typelist<Ts...>, typename repeat_type<(N - 1), Ts...>::type>::type;
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
} // namespace lucid
