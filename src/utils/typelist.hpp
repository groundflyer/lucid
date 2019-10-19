// -*- C++ -*-
// utils.hpp
//

#pragma once

#include <tuple>
#include <utility>
#include <variant>

namespace lucid
{
template <typename Head, typename... Ts>
class typelist
{
    template <std::size_t idx, typename Sought>
    static constexpr std::size_t
    index_impl() noexcept
    {
        const constexpr bool recurse = sizeof...(Ts) > 0ul;

        if constexpr(std::is_same_v<Sought, Head>)
            return idx;
        else if constexpr(recurse)
            return index_impl<idx + 1, Sought, Ts...>();
        else
            static_assert(recurse, "There is no such type in the typelist.");
    }

  public:
    using head    = Head;
    using variant = typename std::variant<head, Ts...>;
    using tuple   = typename std::tuple<head, Ts...>;

    template <template <typename> typename Array,
              template <typename...> typename Tuple = std::tuple>
    using tuple_of_arrays = Tuple<Array<head>, Array<Ts>...>;

    template <std::size_t I>
    using get = std::tuple_element_t<I, tuple>;

    template <typename Type>
    static constexpr std::size_t
    index() noexcept
    {
        return index_impl<0, Type>();
    }

    static const constexpr std::size_t size = sizeof...(Ts) + 1;
    static const constexpr bool        same = (true && ... && std::is_same_v<head, Ts>);

    using indices = std::make_index_sequence<size>;

    template <typename... Args>
    using result_of =
        typelist<std::invoke_result_t<head, Args...>, std::invoke_result_t<Ts, Args...>...>;

    constexpr typelist(std::tuple<Head, Ts...>) noexcept {}

    constexpr typelist(std::variant<Head, Ts...>) noexcept {}
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
