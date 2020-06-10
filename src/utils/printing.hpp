// -*- C++ -*-
// printing.hpp
//

#pragma once

#include <base/matrix.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <iostream>
#include <sstream>
#include <tuple>

namespace lucid
{
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
std::ostream&
operator<<(std::ostream& os, const VectorType<T, N, Container>& rhs) noexcept
{
    os << '[';
    for(std::size_t i = 0; i < N - 1; ++i) os << rhs[i] << ", ";
    os << rhs[N - 1] << ']';

    return os;
}

template <typename T,
          std::size_t R,
          std::size_t C,
          template <typename, std::size_t>
          typename Container>
std::ostream&
operator<<(std::ostream& os, const Matrix<T, R, C, Container>& rhs) noexcept
{
    os << '[';
    for(std::size_t i = 0; i < R - 1; ++i) os << rhs[i] << ", ";
    os << rhs[R - 1] << ']';

    return os;
}
} // namespace lucid

namespace std
{
template <typename T, std::size_t N>
ostream&
operator<<(std::ostream& os, const array<T, N>& rhs) noexcept
{
    os << '[';
    for(std::size_t i = 0; i < N - 1; ++i) os << rhs[i] << ", ";
    os << rhs[N - 1] << ']';
    return os;
}

template <typename Tuple, std::size_t... I>
ostream&
print_tuple(ostream& os, const Tuple& t, index_sequence<I...>)
{
    os << '(';
    (..., (os << (I == 0 ? "" : ", ") << get<I>(t)));
    os << ')';
    return os;
}

template <typename... Ts>
ostream&
operator<<(ostream& os, const tuple<Ts...>& t)
{
    return print_tuple(os, t, make_index_sequence<sizeof...(Ts)>());
}

template <typename T1, typename T2>
ostream&
operator<<(std::ostream& os, pair<T1, T2> arg)
{
    os << '(' << arg.first << ", " << arg.second << ')';
    return os;
}
} // namespace std
