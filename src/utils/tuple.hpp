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

    BinaryOp   op;
    ResultType operand;

    constexpr fold_wrapper(BinaryOp _op, const Operand& _operand) : op(_op), operand(_operand) {}

    constexpr fold_wrapper() = delete;

    constexpr fold_wrapper(const fold_wrapper&) = delete;

    template <typename OperandB>
    constexpr decltype(auto)
    operator%(const fold_wrapper<BinaryOp, OperandB>& rhs) const noexcept
    {
        return fold_wrapper(op, op(operand, rhs.operand));
    }
};

template <typename Tuple, std::size_t... Ids>
decltype(auto)
enumerate_impl(Tuple&& tuple, std::index_sequence<Ids...>) noexcept
{
    return std::tuple{std::pair{Ids, std::get<Ids>(tuple)}...};
}

template <typename Ret, std::size_t Idx, typename TupleArgs, typename Func, typename... RestFuncs>
constexpr Ret
switcher_func_impl(const std::size_t case_,
                   TupleArgs&&       tuple_args,
                   Func&&            func,
                   RestFuncs&&... rest) noexcept
{
    if(Idx == case_ || sizeof...(rest) == 0)
        return std::apply(std::forward<Func>(func), std::forward<TupleArgs>(tuple_args));

    if constexpr(sizeof...(rest) > 0)
        return switcher_func_impl<Ret, Idx + 1>(
            case_, std::forward<TupleArgs>(tuple_args), std::forward<RestFuncs>(rest)...);
}

template <typename Ret, std::size_t Idx, typename Visitor, typename Tuple>
constexpr Ret
visit_impl(const std::size_t case_, Visitor&& visitor, const Tuple& tuple) noexcept
{
    if(Idx == case_ || Idx == std::tuple_size_v<Tuple> - 1) return visitor(std::get<Idx>(tuple));

    if constexpr(Idx < std::tuple_size_v<Tuple> - 1)
        return visit_impl<Ret, Idx + 1>(case_, std::forward<Visitor>(visitor), tuple);
}

template <std::size_t I, typename T, std::size_t N1, std::size_t N2>
constexpr decltype(auto)
array_cat_idx(const std::array<T, N1>& first, const std::array<T, N2>& second)
{
    if constexpr(I < N1)
        return std::get<I>(first);
    else
        return std::get<I - N1>(second);
}

template <typename T, std::size_t N1, std::size_t N2, std::size_t... Is>
constexpr auto
array_cat_impl(const std::array<T, N1>& first,
               const std::array<T, N2>& second,
               std::index_sequence<Is...>) noexcept
{
    return std::array<T, N1 + N2>{array_cat_idx<Is>(first, second)...};
}
} // namespace detail

template <typename BinaryOp, typename Init, typename... Args>
constexpr decltype(auto)
reduce(BinaryOp&& op, Init&& init, Args&&... args) noexcept
{
    return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand;
}

template <typename... Ts>
constexpr decltype(auto)
enumerate(const std::tuple<Ts...>& tuple) noexcept
{
    return detail::enumerate_impl(tuple, std::index_sequence_for<Ts...>{});
}

template <typename... Args, typename... Funcs, template <typename...> typename Tuple = std::tuple>
constexpr decltype(auto)
switcher_func(const std::size_t case_, Tuple<Funcs...>&& funcs, Args&&... args) noexcept
{
    return std::apply(
        [case_, args_tuple = std::forward_as_tuple(args...)](Funcs&&... items) mutable {
            using tpl = typename typelist<Funcs...>::template result_of<Args...>;
            using ret = std::conditional_t<tpl::same, typename tpl::head, typename tpl::variant>;
            return detail::switcher_func_impl<ret, 0>(
                case_, args_tuple, std::forward<decltype(items)>(items)...);
        },
        std::forward<Tuple<Funcs...>>(funcs));
}

template <typename Visitor, typename... Ts, template <typename...> typename Tuple = std::tuple>
constexpr decltype(auto)
visit(const std::size_t case_, Visitor&& visitor, const Tuple<Ts...>& tuple) noexcept
{
    using tpl = typelist<std::invoke_result_t<Visitor, Ts>...>;
    using ret = std::conditional_t<tpl::same, typename tpl::head, typename tpl::variant>;

    return detail::visit_impl<ret, 0>(case_, std::forward<Visitor>(visitor), tuple);
}

template <typename T, std::size_t N1, std::size_t N2, typename... Rest>
constexpr auto
array_cat(const std::array<T, N1>& first,
          const std::array<T, N2>& second,
          const Rest&... rest) noexcept
{
    const auto ret = detail::array_cat_impl(first, second, std::make_index_sequence<N1 + N2>{});

    if constexpr(sizeof...(rest)) return array_cat(ret, rest...);

    return ret;
}
} // namespace lucid
