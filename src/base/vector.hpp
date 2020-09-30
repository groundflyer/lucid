// -*- C++ -*-
// vector.hpp --
//

/// @file
/// Contains the definition of a generic vector with constant dimensionality.

#pragma once

#include "vector_operators.hpp"

#include <utils/debug.hpp>

namespace lucid
{
/// @brief Generic vector with constant dimensionality.
/// @tparam T value type.
/// @tparam N dimensionality.
/// @tparam Container defines how to store values.
template <typename T,
          std::size_t N,
          template <typename, std::size_t> typename Container = std::array>
class Vector :
    public ImmutableVectorOperators<T, N, Container, Vector>,
    public MutableVectorOperators<T, N, Container, Vector>
{
    using Data = Container<T, N>;

    Data m_data{};

  public:
    constexpr Vector() noexcept {}

    constexpr Vector(const Vector& rhs) noexcept : m_data(rhs.m_data) {}

    constexpr Vector(Vector&& rhs) noexcept : m_data(std::move(rhs.m_data)) {}

    explicit constexpr Vector(const Data& rhs) noexcept : m_data(rhs) {}

    explicit constexpr Vector(Data&& rhs) noexcept : m_data(std::move(rhs)) {}

    /// @brief OpenGL-style constructor.
    ///
    /// Allows to construct vector from mix of vectors of different dimensionality
    /// and scalar values.
    template <typename... Ts>
    explicit constexpr Vector(Ts&&... rhs) noexcept :
        m_data(vector_constructor<0>(m_data, std::forward<Ts>(rhs)...))
    {
    }

    constexpr Vector&
    operator=(const Vector& rhs) noexcept
    {
        m_data = rhs.m_data;
        return *this;
    }

    constexpr Vector&
    operator=(Vector&& rhs) noexcept
    {
        m_data = std::move(rhs.m_data);
        return *this;
    }

    constexpr decltype(auto)
    operator=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] = rhs;

        return *this;
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr decltype(auto)
    operator=(const VectorType2<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] = rhs[i];

        return *this;
    }

    constexpr auto
    begin() noexcept
    {
        return m_data.begin();
    }
    constexpr auto
    end() noexcept
    {
        return m_data.end();
    }
    constexpr auto
    cbegin() const noexcept
    {
        return m_data.cbegin();
    }
    constexpr auto
    cend() const noexcept
    {
        return m_data.cend();
    }

    const constexpr T&
    operator[](const std::size_t i) const noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }
    constexpr T&
    operator[](const std::size_t i) noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }

    template <std::size_t I>
    constexpr T&
    get() noexcept
    {
        return std::get<I>(m_data);
    }

    template <std::size_t I>
    constexpr const T&
    get() const noexcept
    {
        return std::get<I>(m_data);
    }

    const constexpr Data&
    data() const noexcept
    {
        return m_data;
    }

    constexpr auto
    size() const noexcept
    {
        return N;
    }
};

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
Vector(Container<T, N> &&) -> Vector<T, N, Container>;
} // namespace lucid

namespace std
{
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
class tuple_size<lucid::Vector<T, N, Container>>
{
  public:
    static const constexpr std::size_t value = N;
};

template <std::size_t I,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
class tuple_element<I, lucid::Vector<T, N, Container>>
{
  public:
    using type = T;
};

template <std::size_t I,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr decltype(auto)
get(const lucid::Vector<T, N, Container>& vec) noexcept
{
    return vec.template get<I>();
}
} // namespace std
