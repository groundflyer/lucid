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
    constexpr decltype(auto)
    operator()(LHS lhs, RHS rhs) const
    {
        return [=](auto&&... args) -> decltype(auto) {
            return std::invoke(lhs, std::invoke(rhs, args...));
        };
    }

    template <typename LHS, typename RHS>
    constexpr decltype(auto)
    operator()(LHS lhs, RHS rhs)
    {
        return [=](auto&&... args) -> decltype(auto) {
            return std::invoke(lhs, std::invoke(rhs, args...));
        };
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
    return [=](auto&& rhs, auto&& lhs) { return std::invoke(f, lhs, rhs); };
}
} // namespace lucid
