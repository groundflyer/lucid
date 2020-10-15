// -*- C++ -*-
// vector.hpp --
//

/// @file
/// Definition of generic N-dimenstional Vector and its functions.

#pragma once

#include <utils/debug.hpp>
#include <utils/math.hpp>
#include <utils/static_span.hpp>

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

namespace lucid
{
/// @brief Generic N-dimnsional euclidian vector.
/// @tparam T value type. Must be arithmetic.
/// @tparam N dimensionality.
/// @tparam Container std::array-like container that used to store the data.
template <typename T, size_t N, template <typename, size_t> typename Container>
class Vector
{
    static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

    using Data = Container<T, N>;

    Data m_data{};

    template <size_t idx>
    constexpr void
    unpack()
    {
        if constexpr(idx < N)
            for(auto i = idx; i < N; ++i) m_data[i] = static_cast<T>(0);
    }

    template <size_t idx, typename... Ts>
    constexpr void
    unpack(const T& first, const Ts&... other)
    {
        static_assert(idx < N, "Too many elements.");
        m_data[idx] = first;
        if constexpr(sizeof...(other) == 0 && idx < N - 1)
            unpack<idx + 1>(first);
        else
            unpack<idx + 1>(other...);
    }

    template <size_t idx,
              typename T1,
              size_t N1,
              template <typename, size_t>
              typename Container1,
              typename... Ts>
    constexpr void
    unpack(const Vector<T1, N1, Container1>& first, const Ts&... other)
    {
        static_assert(idx < N, "Too many elements.");
        for(size_t i = 0; i < std::min(N - idx, N1); ++i)
            m_data[idx + i] = static_cast<T>(first[i]);
        unpack<idx + N1>(other...);
    }

    template <size_t idx, typename T1, size_t N1, typename... Ts>
    constexpr void
    unpack(const Container<T1, N1>& first, const Ts&... other)
    {
        static_assert(idx < N, "Too many elements.");
        for(size_t i = 0; i < std::min(N - idx, N1); ++i)
            m_data[idx + i] = static_cast<T>(first[i]);
        unpack<idx + N1>(other...);
    }

  public:
    constexpr Vector() noexcept {}

    constexpr Vector(const Vector& rhs) noexcept : m_data(rhs.m_data) {}

    explicit constexpr Vector(Data&& rhs) noexcept : m_data(std::move(rhs)) {}

    explicit constexpr Vector(const Data& rhs) noexcept : m_data(rhs) {}

    explicit constexpr Vector(Data& rhs) noexcept : m_data(rhs) {}

    explicit constexpr Vector(T&& rhs) noexcept
    {
        for(T& i: m_data) i = rhs;
    }

    template <typename... Ts>
    explicit constexpr Vector(const Ts&... rhs) noexcept
    {
        unpack<0>(rhs...);
    }

    constexpr Vector&
    operator=(const Vector& rhs) noexcept
    {
        m_data = rhs.m_data;
        return *this;
    }

    // between different containers we copy data manually
    template <template <typename, size_t> typename Container2>
    constexpr Vector&
    operator=(const Vector<T, N, Container2>& rhs) noexcept
    {
        std::copy(rhs.cbegin(), rhs.cend(), begin());
        return *this;
    }

    constexpr Vector&
    operator=(const T& rhs) noexcept
    {
        for(T& i: m_data) i = rhs;
        return *this;
    }

    constexpr auto
    begin() const noexcept
    {
        return m_data.begin();
    }
    constexpr auto
    end() const noexcept
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

    constexpr const T&
    operator[](const size_t i) const noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }
    constexpr T&
    operator[](const size_t i) noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }

    template <size_t I>
    constexpr T&
    get() noexcept
    {
        return std::get<I>(m_data);
    }

    template <size_t I>
    constexpr const T&
    get() const noexcept
    {
        return std::get<I>(m_data);
    }

    constexpr const T&
    at(const size_t i) const noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }
    constexpr T&
    at(const size_t i) noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }

    constexpr const Data&
    data() const noexcept
    {
        return m_data;
    }

    template <template <typename, size_t> typename Container2>
    constexpr auto
    operator+(const Vector<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::plus<T>(), *this, rhs);
    }
    constexpr auto
    operator+(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a + rhs; }, *this);
    }

    template <template <typename, size_t> typename Container2>
    constexpr auto
    operator-(const Vector<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::minus<T>(), *this, rhs);
    }
    constexpr auto
    operator-(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a - rhs; }, *this);
    }
    constexpr auto
    operator-() const noexcept
    {
        return transform(std::negate<T>(), *this);
    }

    template <template <typename, size_t> typename Container2>
    constexpr auto
    operator*(const Vector<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::multiplies<T>(), *this, rhs);
    }
    constexpr auto
    operator*(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a * rhs; }, *this);
    }

    template <template <typename, size_t> typename Container2>
    constexpr auto
    operator/(const Vector<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::divides<T>(), *this, rhs);
    }
    constexpr auto
    operator/(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a / rhs; }, *this);
    }

    template <template <typename, size_t> typename Container2>
    constexpr Vector&
    operator+=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] += rhs[i];

        return *this;
    }
    constexpr Vector&
    operator+=(const T& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] += rhs;

        return *this;
    }

    template <template <typename, size_t> typename Container2>
    constexpr Vector&
    operator-=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] -= rhs[i];

        return *this;
    }
    constexpr Vector&
    operator-=(const T& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] -= rhs;

        return *this;
    }

    template <template <typename, size_t> typename Container2>
    constexpr Vector&
    operator*=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] *= rhs[i];

        return *this;
    }
    constexpr Vector&
    operator*=(const T& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] *= rhs;

        return *this;
    }

    template <template <typename, size_t> typename Container2>
    constexpr Vector&
    operator/=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] /= rhs[i];

        return *this;
    }

    constexpr Vector&
    operator/=(const T& rhs) noexcept
    {
        for(size_t i = 0; i < N; ++i) m_data[i] /= rhs;

        return *this;
    }

    template <template <typename, size_t> typename Container2>
    friend constexpr auto
    operator==(const Vector& lhs, const Vector<T, N, Container2>& rhs) noexcept
    {
        return transform(std::equal_to<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator==(const Vector& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem == rhs; }, lhs);
    }

    template <template <typename, size_t> typename Container2>
    friend constexpr auto
    operator!=(const Vector& lhs, const Vector<T, N, Container2>& rhs) noexcept
    {
        return transform(std::not_equal_to<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator!=(const Vector& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem != rhs; }, lhs);
    }

    template <template <typename, size_t> typename Container2>
    friend constexpr auto
    operator>(const Vector& lhs, const Vector<T, N, Container2>& rhs) noexcept
    {
        return transform(std::greater<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator>(const Vector& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem > rhs; }, lhs);
    }

    template <template <typename, size_t> typename Container2>
    friend constexpr auto
    operator<(const Vector& lhs, const Vector<T, N, Container2>& rhs) noexcept
    {
        return transform(std::less<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator<(const Vector& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem < rhs; }, lhs);
    }

    template <template <typename, size_t> typename Container2>
    friend constexpr auto
    operator>=(const Vector& lhs, const Vector<T, N, Container2>& rhs) noexcept
    {
        return transform(std::greater_equal<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator>=(const Vector& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem >= rhs; }, lhs);
    }

    template <template <typename, size_t> typename Container2>
    friend constexpr auto
    operator<=(const Vector& lhs, const Vector<T, N, Container2>& rhs) noexcept
    {
        return transform(std::less_equal<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator<=(const Vector& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem <= rhs; }, lhs);
    }

    constexpr auto
    operator!() const noexcept
    {
        return transform(std::logical_not<T>(), *this);
    }

    constexpr std::size_t
    size() const noexcept
    {
        return N;
    }
};

template <typename T, size_t N, template <typename, size_t> typename Container>
Vector(Container<T, N> &&) -> Vector<T, N, Container>;

/// @brief Create reference to input vector.
///
/// Creates a such vector object that data it contains is mapped to
/// other place.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr const Vector<T, N, StaticSpan>
ref(const Vector<T, N, Container>& v)
{
    return Vector<T, N, StaticSpan>(StaticSpan<T, N>(v.template get<0>()));
}

/// @brief Apply binary operator to the corresponding elements of input vectors.
/// @return @p vector of VectorType1 with dimensionality @p N and element type @p c.
template <typename T1,
          typename T2,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2,
          typename BinaryOperation>
constexpr auto
transform(BinaryOperation                  binary_op,
          const Vector<T1, N, Container1>& a,
          const Vector<T2, N, Container2>& b) noexcept
{
    Vector<std::decay_t<std::invoke_result_t<BinaryOperation, T1, T2>>, N, std::array> ret{};

    for(size_t i = 0; i < N; ++i) ret[i] = binary_op(a[i], b[i]);

    return ret;
}

/// @brief Apply unary operator to the elements of a vector.
/// @return vector of @p VectorType, dimensionality @p N and element type derived from applying the
/// operator @p unary_op.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container,
          typename UnaryOperation>
constexpr auto
transform(UnaryOperation unary_op, const Vector<T, N, Container>& a) noexcept
{
    Vector<std::decay_t<std::invoke_result_t<UnaryOperation, T>>, N, std::array> ret{};

    for(size_t i = 0; i < N; ++i) ret[i] = unary_op(a[i]);

    return ret;
}

/// @brief Perform left fold on a given vector.
/// @param binary_op folding operator.
/// @param a vector to fold.
/// @param init initial value.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container,
          typename BinaryOperation,
          typename Init>
constexpr auto
reduce(BinaryOperation binary_op, const Vector<T, N, Container>& a, Init init) noexcept
{
    for(size_t i = 0; i < N; ++i) init = binary_op(init, a[i]);

    return init;
}

/// @brief Efficient shortcut for
/// @code{.cpp} reduce(binary_op2, transform(binary_op1, a, b), init) @endcode
/// @param binary_op1 operator for transform.
/// @param binary_op2 operator for reduce.
template <typename T1,
          typename T2,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2,
          typename BinaryOperation1,
          typename BinaryOperation2,
          typename Init>
constexpr auto
transform_reduce(BinaryOperation1                 binary_op1,
                 BinaryOperation2                 binary_op2,
                 const Vector<T1, N, Container1>& a,
                 const Vector<T2, N, Container2>& b,
                 Init                             init) noexcept
{
    for(size_t i = 0; i < N; ++i) init = binary_op2(init, binary_op1(a[i], b[i]));

    return init;
}

/// @brief Efficient shortcut for
/// @code{.cpp} reduce(binary_op, transform(unary_op, a), init) @endcode
/// @param unary_op operator for transform.
/// @param binary_op operator for reduce.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container,
          typename UnaryOperation,
          typename BinaryOperation,
          typename Init>
constexpr auto
transform_reduce(UnaryOperation                 unary_op,
                 BinaryOperation                binary_op,
                 const Vector<T, N, Container>& a,
                 Init                           init) noexcept
{
    for(size_t i = 0; i < N; ++i) init = binary_op(init, unary_op(a[i]));

    return init;
}

/// @brief Compute dot product of the given vectors.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr T
dot(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return transform_reduce(std::multiplies<T>(), std::plus<T>(), a, b, T{0});
}

/// @brief Compute cross product of the given vectors.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr auto
cross(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    Vector<T, N, std::array> ret;

    for(size_t i = 0; i < N; ++i)
        for(size_t j = 0; j < N; ++j)
            for(size_t k = 0; k < N; ++k)
                ret[i] += sgn(std::array<size_t, 3>{i, j, k}) * a[j] * b[k];

    return ret;
}

/// @brief Compute squared length of a vector.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
length2(const Vector<T, N, Container>& a) noexcept
{
    return transform_reduce(pow<2, T>, std::plus<T>(), a, T{0});
}

/// @brief Compute length of a vector.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
length(const Vector<T, N, Container>& a) noexcept
{
    return math::sqrt(length2(a));
}

/// @brief Normalize vector.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr Vector<T, N, std::array>
normalize(const Vector<T, N, Container>& a) noexcept
{
    const T l = length(a);
    if constexpr(std::is_floating_point_v<T>)
        return (almost_equal(l, T{1}, 5) || almost_equal(l, T{0}, 5)) ? a : a / l;
    else
        return (l == 1 && l == 0) ? a : a / l;
}

/// @brief Compute distance between two vectors.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr T
distance(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return length(a - b);
}

/// @brief Sum of all vector elements.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
sum(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::plus<T>(), a, T{0});
}

/// @brief Product of all vector elements.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
product(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::multiplies<T>(), a, T{1});
}

/// @brief Test whether all vector elements evaluate to true.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr bool
all(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::logical_and<bool>(), a, true);
}

/// @brief Test whether any vector element evaluates to true.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr bool
any(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::logical_or<bool>(), a, false);
}

/// @brief Compute average of all vector elements.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
avg(const Vector<T, N, Container>& a) noexcept
{
    return sum(a) / N;
}

/// @brief Get the biggest vector element.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
max(const Vector<T, N, Container>& a) noexcept
{
    return reduce(
        static_cast<const T& (*)(const T&, const T&)>(std::max), a, std::numeric_limits<T>::min());
}

/// @brief Get the smallest vector element.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr T
min(const Vector<T, N, Container>& a) noexcept
{
    return reduce(
        static_cast<const T& (*)(const T&, const T&)>(std::min), a, std::numeric_limits<T>::max());
}

/// @brief Build a vector consisting of the biggest corresponding elements from the input vectors.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr auto
max(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return transform(static_cast<const T& (*)(const T&, const T&)>(std::max), a, b);
}

/// @brief Build a vector consisting of the smallest corresponding elements from the input vectors.
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr auto
min(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return transform(static_cast<const T& (*)(const T&, const T&)>(std::min), a, b);
}

/// @brief Test whether corresponding elements of given vectors are equal
/// taking floating point precision into account.
/// @param ulp @ref ulp
template <typename T,
          size_t N,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2,
          typename ULP>
constexpr auto
almost_equal(const Vector<T, N, Container1>& va, const Vector<T, N, Container2>& vb, const ULP ulp)
{
    return transform([ulp](const T a, const T b) { return almost_equal(a, b, ulp); }, va, vb);
}

/// @brief Test whether elements of given vectors are equal to @p b
/// taking floating point precision into account.
/// @param ulp @ref ulp
template <typename T, size_t N, typename ULP, template <typename, size_t> typename Container1>
constexpr auto
almost_equal(const Vector<T, N, Container1>& va, const T b, const ULP ulp)
{
    return transform([ulp, b](const T a) { return almost_equal(a, b, ulp); }, va);
}

/// @brief Shift vector values into a new range.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr auto
fit(const Vector<T, N, Container>& v, const T minval, const T maxval) noexcept
{
    return transform([minval, maxval](const T& val) { return fit(minval, maxval, val); }, v);
}

/// @brief Compute absolute values of vector elements.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr auto
abs(const Vector<T, N, Container>& v) noexcept
{
    return transform(static_cast<T (*)(T)>(math::abs), v);
}

/// @brief Compute square root of vector elements.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr auto
sqrt(const Vector<T, N, Container>& v) noexcept
{
    return transform(static_cast<T (*)(T)>(math::sqrt), v);
}

/// @brief Rise vector elements to a constant power.
/// @tparam exp power exponent.
template <unsigned exp,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
pow(const Vector<T, N, Container>& v) noexcept
{
    return transform(pow<exp, T>, v);
}

/// @brief Clamp vector elements into a range.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr auto
clamp(const Vector<T, N, Container>& v, const T minval, const T maxval) noexcept
{
    return transform([&](const T val) { return std::clamp(val, minval, maxval); }, v);
}

/// @brief Roll vector elements.
/// @param shift the number of places which elements are shifted.
template <typename T, size_t N, template <typename, size_t> typename Container>
constexpr Vector<T, N, std::array>
roll(const Vector<T, N, Container>& v, const size_t shift) noexcept
{
    Vector<T, N, std::array> ret{};

    for(size_t i = 0; i < N; ++i) ret[i] = v[(i - shift) % N];

    return ret;
}

/// @brief Test whether vector elements are finite.
/// @return vector of bool.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
auto
isfinite(const Vector<T, N, Container>& v) noexcept
{
    return transform(static_cast<bool (*)(T)>(std::isfinite), v);
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr const T&
get_x(const Vector<T, N, Container>& v) noexcept
{
    return v.template get<0>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T&
get_x(Vector<T, N, Container>& v) noexcept
{
    return v.template get<0>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr const T&
get_y(const Vector<T, N, Container>& v) noexcept
{
    return v.template get<1>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T&
get_y(Vector<T, N, Container>& v) noexcept
{
    return v.template get<1>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr const T&
get_z(const Vector<T, N, Container>& v) noexcept
{
    return v.template get<2>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T&
get_z(Vector<T, N, Container>& v) noexcept
{
    return v.template get<2>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr const T&
get_w(const Vector<T, N, Container>& v) noexcept
{
    return v.template get<3>();
}

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T&
get_w(Vector<T, N, Container>& v) noexcept
{
    return v.template get<3>();
}
} // namespace lucid

namespace std
{
template <typename T, size_t N, template <typename, size_t> typename Container>
class tuple_size<lucid::Vector<T, N, Container>>
{
  public:
    static constexpr const size_t value = N;
};

template <size_t I, typename T, size_t N, template <typename, size_t> typename Container>
class tuple_element<I, lucid::Vector<T, N, Container>>
{
  public:
    using type = T;
};

template <size_t I, typename T, size_t N, template <typename, size_t> typename Container>
constexpr decltype(auto)
get(const lucid::Vector<T, N, Container>& vec) noexcept
{
    return vec.template get<I>();
}
} // namespace std
