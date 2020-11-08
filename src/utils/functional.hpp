// -*- C++ -*-
// functional.hpp
//

#pragma once

#include <functional>
#include <type_traits>

#define MK_NAMED_FN_OBJ(NAME, FUNC)                   \
    namespace fn                                      \
    {                                                 \
    struct NAME##_fn                                  \
    {                                                 \
        template <typename... Args>                   \
        constexpr decltype(auto)                      \
        operator()(Args&&... args) const noexcept     \
        {                                             \
            return FUNC(std::forward<Args>(args)...); \
        }                                             \
        template <typename Rhs>                       \
        constexpr decltype(auto)                      \
        operator^(const Rhs& rhs) const noexcept      \
        {                                             \
            return lucid::compose(*this, rhs);        \
        }                                             \
    };                                                \
    }                                                 \
    static constexpr fn::NAME##_fn NAME{};

#define MK_FN_OBJ(FUNC) MK_NAMED_FN_OBJ(FUNC, FUNC)

namespace lucid
{
namespace detail
{
struct ComposeOp
{
    template <typename LHS, typename RHS>
    struct Invoker
    {
        LHS lhs;
        RHS rhs;

        constexpr Invoker() noexcept {}
        constexpr Invoker(const Invoker& other) noexcept : lhs(other.lhs), rhs(other.rhs) {}
        constexpr Invoker(Invoker&& other) noexcept :
            lhs(std::move(other.lhs)), rhs(std::move(other.rhs))
        {
        }
        constexpr Invoker(LHS _lhs, RHS _rhs) noexcept : lhs(_lhs), rhs(_rhs) {}

        constexpr Invoker&
        operator=(const Invoker& other) noexcept
        {
            lhs = other.lhs;
            rhs = other.rhs;
            return *this;
        }

        constexpr Invoker&
        operator=(Invoker&& other) noexcept
        {
            lhs = std::move(other.lhs);
            rhs = std::move(other.rhs);
            return *this;
        }

        template <typename... Args>
        constexpr decltype(auto)
        operator()(Args&&... args) const
        {
            return std::invoke(lhs, std::invoke(rhs, args...));
        }

        template <typename... Args>
        constexpr decltype(auto)
        operator()(Args&&... args)
        {
            return std::invoke(lhs, std::invoke(rhs, args...));
        }
    };

    template <typename LHS, typename RHS>
    constexpr decltype(auto)
    operator()(LHS lhs, RHS rhs) const
    {
        return Invoker(lhs, rhs);
    }
};

template <typename BinaryOp, typename LeftOperand>
struct fold_wrapper
{
    BinaryOp    op;
    LeftOperand operand;

    constexpr fold_wrapper(BinaryOp _op, const LeftOperand& _operand) : op(_op), operand(_operand)
    {
    }

    constexpr fold_wrapper() = delete;

    constexpr fold_wrapper(const fold_wrapper&) = delete;

    template <typename RightOperand>
    constexpr decltype(auto)
    operator%(const fold_wrapper<BinaryOp, RightOperand>& rhs) const noexcept
    {
        return fold_wrapper<BinaryOp, std::invoke_result_t<BinaryOp, LeftOperand, RightOperand>>(
            op, op(operand, rhs.operand));
    }

    template <typename RightOperand>
    constexpr decltype(auto)
    operator%(const fold_wrapper<BinaryOp, RightOperand>& rhs) noexcept
    {
        return fold_wrapper<BinaryOp, std::invoke_result_t<BinaryOp, LeftOperand, RightOperand>>(
            op, op(operand, rhs.operand));
    }
};

template <typename T>
struct is_tuple
{
    static constexpr bool value = false;
};

template <typename T1, typename T2>
struct is_tuple<std::pair<T1, T2>>
{
    static constexpr bool value = true;
};

template <typename... Ts>
struct is_tuple<std::tuple<Ts...>>
{
    static constexpr bool value = true;
};
} // namespace detail

namespace fn
{
struct identity_fn
{
    template <typename T>
    constexpr decltype(auto)
    operator()(T&& t) const noexcept
    {
        return t;
    }
};

template <typename T, bool list_init = true>
struct maker_fn
{
    template <typename... Args>
    constexpr T
    operator()(Args&&... args) const
    {
        if constexpr(list_init)
            return T{args...};
        else
            return T(args...);
    }
};
} // namespace fn

static constexpr fn::identity_fn identity;

template <typename T, bool list_init = false>
static constexpr fn::maker_fn<T, list_init> maker;

template <typename BinaryOp, typename Init, typename... Args>
constexpr decltype(auto)
reduce(BinaryOp&& op, Init&& init, Args&&... args) noexcept
{
    return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand;
}

// compose(a, b, c) = a(b(c()))
template <typename F, typename... Fs>
constexpr decltype(auto)
compose(const F& f, Fs&&... fs)
{
    return reduce(detail::ComposeOp{}, std::forward<F>(f), std::forward<Fs>(fs)...);
}

template <typename BinaryF>
constexpr decltype(auto)
flip(BinaryF f) noexcept
{
    return [=](auto&& rhs, auto&& lhs) constexpr { return std::invoke(f, lhs, rhs); };
}

template <std::size_t Idx, typename Head, typename... Tail>
constexpr decltype(auto)
select(Head&& head, Tail&&... tail) noexcept
{
    static_assert(Idx <= sizeof...(tail), "Index is too big");

    if constexpr(Idx == 0ul)
        return head;
    else
        return select<Idx - 1ul>(tail...);
}

template <typename F, typename Arg>
constexpr decltype(auto)
maybe_apply(F&& f, Arg&& arg)
{
    if constexpr(detail::is_tuple<std::decay_t<Arg>>::value)
        return std::apply(f, arg);
    else
        return std::invoke(f, arg);
}
} // namespace lucid
