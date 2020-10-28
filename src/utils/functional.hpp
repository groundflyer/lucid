// -*- C++ -*-
// functional.hpp
//

#pragma once

#include <functional>
#include <type_traits>

#define MK_FN_OBJ(FUNC)                               \
    namespace fn                                      \
    {                                                 \
    struct FUNC##_fn                                  \
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
    static constexpr fn::FUNC##_fn FUNC{};

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
} // namespace detail

namespace fn
{
struct identity_fn
{
    template <typename T>
    constexpr T&
    operator()(T& t) const noexcept
    {
        return t;
    }

    template <typename T>
    constexpr const T&
    operator()(const T& t) const noexcept
    {
        return t;
    }
};
} // namespace fn

static constexpr fn::identity_fn identity;

template <typename BinaryOp, typename Init, typename... Args>
constexpr decltype(auto)
reduce(BinaryOp&& op, Init&& init, Args&&... args) noexcept
{
    return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand;
}

// compose(a, b, c) = a(b(c()))
template <typename F, typename... Fs>
constexpr decltype(auto)
compose(F&& f, Fs&&... fs)
{
    return reduce(detail::ComposeOp{}, std::forward<F>(f), std::forward<Fs>(fs)...);
}

template <typename BinaryF>
constexpr decltype(auto)
flip(BinaryF f) noexcept
{
    return [=](auto&& rhs, auto&& lhs) constexpr { return std::invoke(f, lhs, rhs); };
}

template <typename T, bool list_init = false>
constexpr auto
maker() noexcept
{
    return []<typename... Args>(Args && ... args) constexpr
    {
        if constexpr(list_init)
            return T{std::forward<Args>(args)...};
        else
            return T(std::forward<Args>(args)...);
    };
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
} // namespace lucid
