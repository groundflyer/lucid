// -*- C++ -*-
// seq.hpp
//

#pragma once

#include "typelist.hpp"

#include <type_traits>

namespace lucid
{
    namespace detail
    {
        template <typename BinaryOp, typename Operand>
        struct fold_wrapper
        {
            using ResultType = typename std::result_of_t<BinaryOp(const Operand&, const Operand&)>;

            BinaryOp op;
            ResultType operand;

            constexpr
            fold_wrapper(BinaryOp _op,
                         const Operand& _operand) : op(_op), operand(_operand) {}

            constexpr
            fold_wrapper() = delete;

            constexpr
            fold_wrapper(const fold_wrapper&) = delete;

            template <typename OperandB>
            constexpr decltype(auto)
            operator%(const fold_wrapper<BinaryOp, OperandB>& rhs) const noexcept
            { return fold_wrapper(op, op(operand, rhs.operand)); }
        };

        template <typename Tuple, std::size_t ... Ids>
        decltype(auto)
        enumerate_impl(Tuple&& tuple, std::index_sequence<Ids...>) noexcept
        { return std::tuple{std::pair{Ids, std::get<Ids>(tuple)}...}; }

        template <typename TupleArgs, typename ... Fncs>
        struct
        switcher_ret
        {
            using types = typelist<std::decay_t<decltype(std::apply(std::declval<Fncs>().second, std::declval<TupleArgs>()))>...>;
        };

        template <typename Ret, typename Idx, typename EnumItem, typename TupleArgs, typename ... RestItems>
        constexpr Ret
        switcher_impl(const Idx lookup, TupleArgs&& tuple_args, EnumItem&& enum_item, RestItems&& ... rest) noexcept
        {
            auto&& [idx, item] = enum_item;
            if (idx == lookup || sizeof...(rest) == 0)
                return std::apply(std::forward<decltype(item)>(item), std::forward<TupleArgs>(tuple_args));

            if constexpr (sizeof...(rest) > 0)
                return switcher_impl<Ret>(lookup, std::forward<TupleArgs>(tuple_args), std::forward<RestItems>(rest)...);
        }

        template <std::size_t I, typename T, std::size_t N1, std::size_t N2>
        constexpr decltype(auto)
        array_cat_idx(const std::array<T, N1>& first,
                      const std::array<T, N2>& second)
        {
            if constexpr (I < N1)
                return std::get<I>(first);
            else
                return std::get<I - N1>(second);
        }

        template <typename T, std::size_t N1, std::size_t N2, std::size_t ... Is>
        constexpr auto
        array_cat_impl(const std::array<T, N1>& first,
                       const std::array<T, N2>& second,
                       std::index_sequence<Is...>) noexcept
        { return std::array<T, N1 + N2>{array_cat_idx<Is>(first, second)...}; }
    }

    template <typename BinaryOp, typename Init, typename ... Args>
    constexpr decltype(auto)
    reduce(BinaryOp&& op, Init&& init, Args&&... args) noexcept
    { return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand; }

    template <typename Tuple>
    constexpr decltype(auto)
    enumerate(Tuple&& tuple) noexcept
    { return detail::enumerate_impl(std::forward<Tuple>(tuple), std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}); }

    template <typename Idx, typename Tuple, typename ... Args>
    constexpr decltype(auto)
    switcher(const Idx idx, Tuple&& tuple, const Args& ... args) noexcept
    {
        return std::apply([lookup = idx, args_tuple = std::forward_as_tuple(args...)](auto &&... items) mutable
                        {
                            using tpl = typename detail::switcher_ret<decltype(args_tuple), decltype(items)...>::types;
                            using ret = std::conditional_t<tpl::same, typename tpl::head, typename tpl::variant>;
                            return detail::switcher_impl<ret>(lookup, args_tuple, std::forward<decltype(items)>(items)...);
                        },
          enumerate(std::forward<Tuple>(tuple)));
    }

    template <typename T, std::size_t N1, std::size_t N2, typename ... Rest>
    constexpr auto
    array_cat(const std::array<T, N1>& first,
              const std::array<T, N2>& second,
              const Rest& ... rest) noexcept
    {
        const auto ret = detail::array_cat_impl(first, second,
                                                std::make_index_sequence<N1 + N2>{});

        if constexpr (sizeof...(rest))
            return array_cat(ret, rest...);

        return ret;
    }
}
