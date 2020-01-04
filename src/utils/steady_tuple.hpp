// -*- C++ -*-
// steady_tuple.hpp
//

#pragma once

#include <utils/typelist.hpp>

namespace lucid
{
struct uniform_init
{
};

namespace detail
{
template <size_t I, typename T>
class steady_tuple_leaf
{
    T value;

  public:
    steady_tuple_leaf()                         = delete;
    steady_tuple_leaf(const steady_tuple_leaf&) = delete;
    steady_tuple_leaf(steady_tuple_leaf&&)      = delete;
    steady_tuple_leaf&
    operator=(const steady_tuple_leaf&) = delete;
    steady_tuple_leaf&
    operator=(steady_tuple_leaf&&) = delete;

    template <typename ArgsTuple>
    constexpr steady_tuple_leaf(std::piecewise_construct_t, ArgsTuple&& args) :
        value(std::make_from_tuple<T>(std::forward<ArgsTuple>(args)))
    {
    }

    template <typename... Args>
    constexpr steady_tuple_leaf(Args&&... args) : value(std::forward<Args>(args)...)
    {
    }

    constexpr T&
    get() noexcept
    {
        return value;
    }

    const constexpr T&
    get() const noexcept
    {
        return value;
    }
};

template <typename Idxs, typename... Ts>
class steady_tuple_impl;

template <size_t... Idxs, typename... Ts>
class steady_tuple_impl<std::index_sequence<Idxs...>, Ts...> : public steady_tuple_leaf<Idxs, Ts>...
{
  public:
    steady_tuple_impl()                         = delete;
    steady_tuple_impl(const steady_tuple_impl&) = delete;
    steady_tuple_impl(steady_tuple_impl&&)      = delete;
    steady_tuple_impl&
    operator=(const steady_tuple_impl&) = delete;
    steady_tuple_impl&
    operator=(steady_tuple_impl&&) = delete;

    template <size_t... LeafIdxs, typename... LeafTs, typename... ValueTs>
    constexpr steady_tuple_impl(std::index_sequence<LeafIdxs...>,
                                typelist<LeafTs...>,
                                ValueTs&&... values) :
        steady_tuple_leaf<LeafIdxs, LeafTs>(std::forward<ValueTs>(values))...
    {
    }

    template <size_t... LeafIdxs, typename... LeafTs, typename... ValueTs>
    constexpr steady_tuple_impl(std::index_sequence<LeafIdxs...>,
                                typelist<LeafTs...>,
                                std::piecewise_construct_t pc,
                                ValueTs&&... values) :
        steady_tuple_leaf<LeafIdxs, LeafTs>(pc, std::forward<ValueTs>(values))...
    {
    }

    template <size_t... LeafIdxs, typename... LeafTs, typename Val>
    constexpr steady_tuple_impl(std::index_sequence<LeafIdxs...>,
                                typelist<LeafTs...>,
                                uniform_init,
                                Val&& val) :
        steady_tuple_leaf<LeafIdxs, LeafTs>(std::forward<Val>(val))...
    {
    }
};
} // namespace detail

template <typename... Ts>
class steady_tuple
{
    using Tl   = typelist<Ts...>;
    using Idxs = typename Tl::indices;
    using Data = detail::steady_tuple_impl<Idxs, Ts...>;

    Data data;

  public:
    steady_tuple()                    = delete;
    steady_tuple(const steady_tuple&) = delete;
    steady_tuple(steady_tuple&&)      = delete;
    steady_tuple&
    operator=(const steady_tuple&) = delete;
    steady_tuple&
    operator=(steady_tuple&&) = delete;

    template <typename... Args>
    constexpr steady_tuple(Args&&... args) : data(Idxs{}, Tl{}, std::forward<Args>(args)...)
    {
    }

    template <typename... Args>
    constexpr steady_tuple(std::piecewise_construct_t pc, Args&&... args) :
        data(Idxs{}, Tl{}, pc, std::forward<Args>(args)...)
    {
    }

    template <typename... Args>
    constexpr steady_tuple(uniform_init ui, Args&&... args) :
        data(Idxs{}, Tl{}, ui, std::forward<Args>(args)...)
    {
    }

    template <size_t I>
    constexpr decltype(auto)
    get() const noexcept
    {
        return static_cast<const detail::steady_tuple_leaf<I, typename Tl::template at<I>>&>(data)
            .get();
    }

    template <size_t I>
    constexpr decltype(auto)
    get() noexcept
    {
        return static_cast<detail::steady_tuple_leaf<I, typename Tl::template at<I>>&>(data).get();
    }

    template <typename T>
    constexpr decltype(auto)
    get() const noexcept
    {
        return static_cast<detail::steady_tuple_leaf<Tl::template index<T>(), T>&>(data).get();
    }

    template <typename T>
    constexpr decltype(auto)
    get() noexcept
    {
        return static_cast<detail::steady_tuple_leaf<Tl::template index<T>(), T>&>(data).get();
    }
};
} // namespace lucid
