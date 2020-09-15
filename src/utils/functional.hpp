// -*- C++ -*-
// functional.hpp
//

#pragma once

#include "tuple.hpp"

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
} // namespace detail

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
} // namespace lucid
