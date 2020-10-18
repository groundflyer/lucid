// -*- C++ -*-
// typeinfo.hpp
//

#pragma once

#include <base/matrix.hpp>
#include <base/vector.hpp>

#include <fmt/format.h>

using namespace fmt::literals;

namespace lucid
{
#define MAKE_TYPEINFO_STRING(TYPE) \
    constexpr auto get_typeinfo_string(TYPE) noexcept { return #TYPE; }

MAKE_TYPEINFO_STRING(int)
MAKE_TYPEINFO_STRING(bool)
MAKE_TYPEINFO_STRING(char)
MAKE_TYPEINFO_STRING(long)
MAKE_TYPEINFO_STRING(float)
MAKE_TYPEINFO_STRING(double)
MAKE_TYPEINFO_STRING(size_t)
MAKE_TYPEINFO_STRING(long double)
MAKE_TYPEINFO_STRING(unsigned int)
MAKE_TYPEINFO_STRING(unsigned char)
MAKE_TYPEINFO_STRING(unsigned short)

template <typename T, size_t N>
constexpr auto get_typeinfo_string(lucid::Vector<T, N, std::array>)
{
    return "Vector<{}, {}>"_format(get_typeinfo_string(T{}), N);
}

template <typename T, size_t M, size_t N>
constexpr auto get_typeinfo_string(lucid::Matrix<T, M, N, std::array>)
{
    return "Matrix<{}, {}, {}>"_format(get_typeinfo_string(T{}), M, N);
}

template <typename... Ts>
constexpr auto get_typeinfo_string(std::tuple<Ts...>) noexcept
{
    return "{}"_format(tuple(get_typeinfo_string(Ts{})...));
}

template <typename T, size_t N>
auto get_typeinfo_string(std::array<T, N>) noexcept
{
    return "std::array<{}, {}>"_format(get_typeinfo_string(T{}), N);
}
} // namespace lucid
