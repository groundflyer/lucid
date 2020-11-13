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
            return compose(*this, rhs);               \
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
            return lhs(rhs(std::forward<Args>(args)...));
        }

        template <typename... Args>
        constexpr decltype(auto)
        operator()(Args&&... args)
        {
            return lhs(rhs(std::forward<Args>(args)...));
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
};

template <typename T>
struct maybe_apply_helper
{
    static constexpr bool is_tuple = false;

    template <typename F>
    static constexpr bool is_invocable =
        std::is_invocable_v<F, T> || std::is_nothrow_invocable_v<F, T>;
};

template <typename T1, typename T2>
struct maybe_apply_helper<std::pair<T1, T2>>
{
    static constexpr bool is_tuple = true;

    template <typename F>
    static constexpr bool is_invocable =
        std::is_invocable_v<F, T1, T2> || std::is_nothrow_invocable_v<F, T1, T2>;
};

template <typename... Ts>
struct maybe_apply_helper<std::tuple<Ts...>>
{
    static constexpr bool is_tuple = true;

    template <typename F>
    static constexpr bool is_invocable =
        std::is_invocable_v<F, Ts...> || std::is_nothrow_invocable_v<F, Ts...>;
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

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};

template <typename T, bool list_init = true>
struct maker_fn
{
    template <typename... Args>
    constexpr T
    operator()(Args&&... args) const noexcept
    {
        if constexpr(list_init)
            return T{args...};
        else
            return T(args...);
    }

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};

template <typename T>
struct tuple_maker_fn
{
    template <typename Tuple>
    constexpr T
    operator()(Tuple&& t) const noexcept
    {
        return std::make_from_tuple<T>(t);
    }

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};
} // namespace fn

static constexpr fn::identity_fn identity;

template <typename T, bool list_init = false>
static constexpr fn::maker_fn<T, list_init> maker;

template <typename T>
static constexpr fn::tuple_maker_fn<T> tuple_maker;

template <typename BinaryOp, typename Init, typename... Args>
constexpr decltype(auto)
fold(BinaryOp&& op, Init&& init, Args&&... args) noexcept
{
    return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand;
}

// compose(a, b, c) = a(b(c()))
template <typename... Fs>
constexpr decltype(auto)
compose(Fs&&... fs)
{
    return fold(detail::ComposeOp{}, std::forward<Fs>(fs)...);
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
    using helper = detail::maybe_apply_helper<std::decay_t<Arg>>;
    if constexpr(helper::is_tuple && helper::template is_invocable<F>)
        return std::apply(f, arg);
    else
        return f(std::forward<Arg>(arg));
}
} // namespace lucid
