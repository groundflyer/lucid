// -*- C++ -*-
// seq.hpp
//

#pragma once

#include <type_traits>
#include <tuple>

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

        template <typename EnumItem, typename TupleArgs, typename ... RestItems>
        constexpr decltype(auto)
        switcher_impl(const std::size_t lookup, TupleArgs&& tuple_args, EnumItem&& enum_item, RestItems&& ... rest) noexcept
        {
            auto&& [idx, item] = enum_item;
            if (idx == lookup || sizeof...(rest) == 0)
                return std::apply(std::forward<decltype(item)>(item), std::forward<TupleArgs>(tuple_args));

            if constexpr (sizeof...(rest) > 0)
                return switcher_impl(lookup, std::forward<TupleArgs>(tuple_args), std::forward<RestItems>(rest)...);
        }

        template <std::size_t I, typename T, std::size_t N1, std::size_t N2>
        constexpr decltype(auto)
        array_cat_idx(std::array<T, N1>&& first, std::array<T, N2>&& second)
        {
            if constexpr (I < N1)
                return std::get<I>(std::forward<std::array<T, N1>>(first));
            else
                return std::get<I - N1>(std::forward<std::array<T, N2>>(second));
        }

        template <typename T, std::size_t N1, std::size_t N2, std::size_t ... Is>
        constexpr auto
        array_cat_impl(std::array<T, N1>&& first, std::array<T, N2>&& second, std::index_sequence<Is...>) noexcept
        { return std::array<T, N1 + N2>{std::move(array_cat_idx<Is>(std::forward<std::array<T, N1>>(first),
                                                                    std::forward<std::array<T, N2>>(second)))...}; }
    }

    template <typename BinaryOp, typename Init, typename ... Args>
    constexpr decltype(auto)
    reduce(BinaryOp&& op, Init&& init, Args&&... args) noexcept
    { return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand; }

    template <typename Tuple>
    constexpr decltype(auto)
    enumerate(Tuple&& tuple) noexcept
    { return detail::enumerate_impl(std::forward<Tuple>(tuple), std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}); }

    template <typename Tuple, typename ... Args>
    constexpr decltype(auto)
    switcher(const std::size_t idx, Tuple&& tuple, Args&& ... args) noexcept
    {
        return std::apply([lookup=idx, args_tuple=std::forward_as_tuple(args...)](auto&& ... items) mutable
                          {
                              return detail::switcher_impl(lookup,
                                                           args_tuple,
                                                           std::forward<decltype(items)>(items)...);
                          },
            enumerate(std::forward<Tuple>(tuple)));
    }

    template <typename T, std::size_t N1, std::size_t N2, typename ... Rest>
    constexpr auto
    array_cat(std::array<T, N1>&& first, std::array<T, N2>&& second, Rest&& ... rest) noexcept
    {
        const auto ret = detail::array_cat_impl(std::forward<std::array<T, N1>>(first),
                                                std::forward<std::array<T, N2>>(second),
                                                std::make_index_sequence<N1+N2>{});

        if constexpr (sizeof...(rest))
            return array_cat(ret, rest...);

        return ret;
    }

}
