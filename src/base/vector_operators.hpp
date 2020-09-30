// -*- C++ -*-
// vector.hpp --
//

/// @file
/// Defines common functions for generic vector type.

#pragma once

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
/// @brief Apply binary operator to the corresponding elements of input vectors.
/// @return @p vector of VectorType1 with dimensionality @p N and element type @p c.
template <typename T1,
          typename T2,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2,
          typename BinaryOperation>
constexpr auto
transform(BinaryOperation&&                     binary_op,
          const VectorType1<T1, N, Container1>& a,
          const VectorType2<T2, N, Container2>& b) noexcept
{
    using RetElemType = typename std::decay_t<std::invoke_result_t<BinaryOperation, T1, T2>>;
    using RetType     = VectorType1<RetElemType, N, std::array>;

    RetType ret{};

    for(std::size_t i = 0; i < N; ++i) ret[i] = std::invoke(binary_op, a[i], b[i]);

    return ret;
}

/// @brief Apply unary operator to the elements of a vector.
/// @return vector of @p VectorType, dimensionality @p N and element type derived from applying the
/// operator @p unary_op.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType,
          typename UnaryOperation>
constexpr auto
transform(UnaryOperation&& unary_op, const VectorType<T, N, Container>& a) noexcept
{
    using RetElemType = typename std::decay_t<std::invoke_result_t<UnaryOperation, T>>;
    using RetType     = VectorType<RetElemType, N, std::array>;

    RetType ret{};

    for(std::size_t i = 0; i < N; ++i) ret[i] = std::invoke(unary_op, a[i]);

    return ret;
}

/// @brief Perform left fold on a given vector.
/// @param binary_op folding operator.
/// @param a vector to fold.
/// @param init initial value.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType,
          typename BinaryOperation,
          typename Init>
constexpr Init
reduce(BinaryOperation&& binary_op, const VectorType<T, N, Container>& a, Init init) noexcept
{
    for(std::size_t i = 0; i < N; ++i) init = std::invoke(binary_op, init, a[i]);

    return init;
}

/// @brief Efficient shortcut for
/// @code{.cpp} reduce(binary_op2, transform(binary_op1, a, b), init) @endcode
/// @param binary_op1 operator for transform.
/// @param binary_op2 operator for reduce.
template <typename T1,
          typename T2,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2,
          typename BinaryOperation1,
          typename BinaryOperation2,
          typename Init>
constexpr Init
transform_reduce(BinaryOperation1&&                    binary_op1,
                 BinaryOperation2&&                    binary_op2,
                 const VectorType1<T1, N, Container1>& a,
                 const VectorType2<T2, N, Container2>& b,
                 Init                                  init) noexcept
{
    for(std::size_t i = 0; i < N; ++i)
        init = std::invoke(binary_op2, init, std::invoke(binary_op1, a[i], b[i]));

    return init;
}

/// @brief Efficient shortcut for
/// @code{.cpp} reduce(binary_op, transform(unary_op, a), init) @endcode
/// @param unary_op operator for transform.
/// @param binary_op operator for reduce.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType,
          typename UnaryOperation,
          typename BinaryOperation,
          typename Init>
constexpr Init
transform_reduce(UnaryOperation&&                   unary_op,
                 BinaryOperation&&                  binary_op,
                 const VectorType<T, N, Container>& a,
                 Init                               init) noexcept
{
    for(std::size_t i = 0; i < N; ++i) init = std::invoke(binary_op, init, unary_op(a[i]));

    return init;
}

/// @brief Compute squared length of a vector.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
length2(const VectorType<T, N, Container>& a) noexcept
{
    return transform_reduce(pow<2, T>, std::plus<T>(), a, T{0});
}

/// @brief Compute length of a vector.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
length(const VectorType<T, N, Container>& a) noexcept
{
    return math::sqrt(length2(a));
}

/// @brief Normalize vector.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
normalize(const VectorType<T, N, Container>& a) noexcept
{
    const T l = length(a);
    if constexpr(std::is_floating_point_v<T>)
        return !(almost_equal(l, T{1}, 5) || almost_equal(l, T{0}, 5)) ? a / l : a;
    else
        return !(l == 1 && l == 0) ? a / l : a;
}

/// @brief Compute distance between two vectors.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2>
constexpr T
distance(const VectorType1<T, N, Container1>& a, const VectorType2<T, N, Container2>& b)
{
    return length(a - b);
}

/// @brief Sum of all vector elements.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
sum(const VectorType<T, N, Container>& a) noexcept
{
    return reduce(std::plus<T>(), a, T{0});
}

/// @brief Product of all vector elements.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
product(const VectorType<T, N, Container>& a) noexcept
{
    return reduce(std::multiplies<T>(), a, T{1});
}

/// @brief Test whether all vector elements evaluate to true.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr bool
all(const VectorType<T, N, Container>& a) noexcept
{
    return reduce(std::logical_and<bool>(), a, true);
}

/// @brief Test whether any vector element evaluates to true.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr bool
any(const VectorType<T, N, Container>& a) noexcept
{
    return reduce(std::logical_or<bool>(), a, false);
}

/// @brief Compute average of all vector elements.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
avg(const VectorType<T, N, Container>& a) noexcept
{
    return sum(a) / T{N};
}

/// @brief Get the biggest vector element.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
max(const VectorType<T, N, Container>& a) noexcept
{
    return reduce(
        static_cast<const T& (*)(const T&, const T&)>(std::max), a, std::numeric_limits<T>::min());
}

/// @brief Get the smallest vector element.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T
min(const VectorType<T, N, Container>& a) noexcept
{
    return reduce(
        static_cast<const T& (*)(const T&, const T&)>(std::min), a, std::numeric_limits<T>::max());
}

/// @brief Build a vector consisting of the biggest corresponding elements from the input vectors.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2>
constexpr auto
max(const VectorType1<T, N, Container1>& a, const VectorType2<T, N, Container2>& b) noexcept
{
    return transform(static_cast<const T& (*)(const T&, const T&)>(std::max), a, b);
}

/// @brief Build a vector consisting of the smallest corresponding elements from the input vectors.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2>
constexpr auto
min(const VectorType1<T, N, Container1>& a, const VectorType2<T, N, Container2>& b) noexcept
{
    return transform(static_cast<const T& (*)(const T&, const T&)>(std::min), a, b);
}

/// @brief Test whether corresponding elements of given vectors are equal
/// taking floating point precision into account.
/// @param ulp @ref ulp
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2,
          typename ULP>
constexpr auto
almost_equal(const VectorType1<T, N, Container1>& va,
             const VectorType2<T, N, Container2>& vb,
             const ULP                            ulp)
{
    return transform([ulp](const T a, const T b) { return almost_equal(a, b, ulp); }, va, vb);
}

/// @brief Test whether elements of given vectors are equal to @p b
/// taking floating point precision into account.
/// @param ulp @ref ulp
template <typename T,
          std::size_t N,
          typename ULP,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
almost_equal(const VectorType<T, N, Container>& va, const T b, const ULP ulp)
{
    return transform([ulp, b](const T a) { return almost_equal(a, b, ulp); }, va);
}

/// @brief Shift vector values into a new range.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
fit(const VectorType<T, N, Container>& v, const T minval, const T maxval) noexcept
{
    return transform([minval, maxval](const T& val) { return fit(minval, maxval, val); }, v);
}

/// @brief Compute absolute values of vector elements.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
abs(const VectorType<T, N, Container>& v) noexcept
{
    return transform(static_cast<T (*)(T)>(math::abs), v);
}

/// @brief Compute square root of vector elements.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
sqrt(const VectorType<T, N, Container>& v) noexcept
{
    return transform(static_cast<T (*)(T)>(math::sqrt), v);
}

/// @brief Rise vector elements to a constant power.
/// @tparam exp power exponent.
template <unsigned exp,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
pow(const VectorType<T, N, Container>& v) noexcept
{
    return transform(pow<exp, T>, v);
}

/// @brief Clamp vector elements into a range.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
clamp(const VectorType<T, N, Container>& v, const T minval, const T maxval) noexcept
{
    return transform([&](const T val) { return std::clamp(val, minval, maxval); }, v);
}

/// @brief Roll vector elements.
/// @param shift the number of places which elements are shifted.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
roll(const VectorType<T, N, Container>& v, const std::size_t shift) noexcept
{
    VectorType<T, N, std::array> ret{};

    for(std::size_t i = 0; i < N; ++i) ret[i] = v[(i - shift) % N];

    return ret;
}

/// @brief Test whether vector elements are finite.
/// @return vector of bool.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
auto
isfinite(const VectorType<T, N, Container>& v) noexcept
{
    return transform(static_cast<bool (*)(T)>(std::isfinite), v);
}

/// @brief Compute dot product of the given vectors.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2>
constexpr auto
dot(const VectorType1<T, N, Container1>& a, const VectorType2<T, N, Container2>& b) noexcept
{
    return transform_reduce(std::multiplies<T>(), std::plus<T>(), a, b, T{0});
}

/// @brief Compute cross product of the given vectors.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType1,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType2>
constexpr auto
cross(const VectorType1<T, N, Container1>& a, const VectorType2<T, N, Container2>& b) noexcept
{
    VectorType1<T, N, std::array> ret{};

    for(std::size_t i = 0; i < N; ++i)
        for(std::size_t j = 0; j < N; ++j)
            for(std::size_t k = 0; k < N; ++k)
                ret[i] += sgn(std::array<std::size_t, 3>({{i, j, k}})) * a[j] * b[k];

    return ret;
}

/// @brief Build a vector view which elements reference to the elements of the given vector.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
ref(const VectorType<T, N, Container>& v) noexcept
{
    return VectorType<T, N, StaticSpan>(StaticSpan<T, N>(v[0]));
}

/// @brief Compute sRGB luminance.
///
/// https://en.wikipedia.org/wiki/Relative_luminance
template <typename T,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr auto
srgb_luminance(const VectorType<T, 3, Container>& rgb) noexcept
{
    return rgb.dot(VectorType<T, 3, std::array>{T{0.2126}, T{0.7152}, T{0.0722}});
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr const T&
get_x(const VectorType<T, N, Container>& v) noexcept
{
    return v.template get<0>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T&
get_x(VectorType<T, N, Container>& v) noexcept
{
    return v.template get<0>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr const T&
get_y(const VectorType<T, N, Container>& v) noexcept
{
    return v.template get<1>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T&
get_y(VectorType<T, N, Container>& v) noexcept
{
    return v.template get<1>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr const T&
get_z(const VectorType<T, N, Container>& v) noexcept
{
    return v.template get<2>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T&
get_z(VectorType<T, N, Container>& v) noexcept
{
    return v.template get<2>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr const T&
get_w(const VectorType<T, N, Container>& v) noexcept
{
    return v.template get<3>();
}

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
constexpr T&
get_w(VectorType<T, N, Container>& v) noexcept
{
    return v.template get<3>();
}

/// @brief Meta-class defining common vector operators that don't modify values in-place.
///
/// @note
/// Comparison operators defined here are numpy-like: they perform comparison on
/// all the elements returning vector of bool instead of commonly used lexicographic order.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
class ImmutableVectorOperators
{
    static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

    constexpr const VectorType<T, N, Container>&
    this_vec() const noexcept
    {
        return static_cast<const VectorType<T, N, Container>&>(*this);
    }

  public:
    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr auto
    operator+(const VectorType2<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::plus<T>(), this_vec(), rhs);
    }
    constexpr auto
    operator+(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a + rhs; }, this_vec());
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr auto
    operator-(const VectorType2<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::minus<T>(), this_vec(), rhs);
    }
    constexpr auto
    operator-(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a - rhs; }, this_vec());
    }
    constexpr auto
    operator-() const noexcept
    {
        return transform(std::negate<T>(), this_vec());
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr auto
    operator*(const VectorType2<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::multiplies<T>(), this_vec(), rhs);
    }
    constexpr auto
    operator*(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a * rhs; }, this_vec());
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr auto
    operator/(const VectorType2<T, N, Container2>& rhs) const noexcept
    {
        return transform(std::divides<T>(), this_vec(), rhs);
    }
    constexpr auto
    operator/(const T& rhs) const noexcept
    {
        return transform([&rhs](const T& a) { return a / rhs; }, this_vec());
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    friend constexpr auto
    operator==(const VectorType<T, N, Container>&   lhs,
               const VectorType2<T, N, Container2>& rhs) noexcept
    {
        return transform(std::equal_to<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator==(const VectorType<T, N, Container>& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem == rhs; }, lhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    friend constexpr auto
    operator!=(const VectorType<T, N, Container>&   lhs,
               const VectorType2<T, N, Container2>& rhs) noexcept
    {
        return transform(std::not_equal_to<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator!=(const VectorType<T, N, Container>& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem != rhs; }, lhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    friend constexpr auto
    operator>(const VectorType<T, N, Container>&   lhs,
              const VectorType2<T, N, Container2>& rhs) noexcept
    {
        return transform(std::greater<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator>(const VectorType<T, N, Container>& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem > rhs; }, lhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    friend constexpr auto
    operator<(const VectorType<T, N, Container>&   lhs,
              const VectorType2<T, N, Container2>& rhs) noexcept
    {
        return transform(std::less<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator<(const VectorType<T, N, Container>& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem < rhs; }, lhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    friend constexpr auto
    operator>=(const VectorType<T, N, Container>&   lhs,
               const VectorType2<T, N, Container2>& rhs) noexcept
    {
        return transform(std::greater_equal<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator>=(const VectorType<T, N, Container>& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem >= rhs; }, lhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    friend constexpr auto
    operator<=(const VectorType<T, N, Container>&   lhs,
               const VectorType2<T, N, Container2>& rhs) noexcept
    {
        return transform(std::less_equal<T>(), lhs, rhs);
    }
    friend constexpr auto
    operator<=(const VectorType<T, N, Container>& lhs, const T& rhs) noexcept
    {
        return transform([&rhs](const T& elem) { return elem <= rhs; }, lhs);
    }

    friend constexpr auto
    operator!(const VectorType<T, N, Container>& lhs) noexcept
    {
        return transform(std::logical_not<T>(), lhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr auto
    dot(const VectorType2<T, N, Container2>& rhs) const noexcept
    {
        return lucid::dot(this_vec(), rhs);
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr auto
    cross(const VectorType2<T, N, Container2>& rhs) const noexcept
    {
        return lucid::cross(this_vec(), rhs);
    }
};

/// @brief Meta-class defining operators that modify vector elements in-place, such as +=, *=, etc.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType>
class MutableVectorOperators
{
    static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

    constexpr VectorType<T, N, Container>&
    this_vec() noexcept
    {
        return static_cast<VectorType<T, N, Container>&>(*this);
    }

  public:
    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr decltype(auto)
    operator+=(const VectorType2<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] += rhs[i];

        return this_vec();
    }
    constexpr decltype(auto)
    operator+=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] += rhs;

        return this_vec();
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr decltype(auto)
    operator-=(const VectorType2<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] -= rhs[i];

        return this_vec();
    }
    constexpr decltype(auto)
    operator-=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] -= rhs;

        return this_vec();
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr decltype(auto)
    operator*=(const VectorType2<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] *= rhs[i];

        return this_vec();
    }
    constexpr decltype(auto)
    operator*=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] *= rhs;

        return this_vec();
    }

    template <template <typename, std::size_t> typename Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename>
              typename VectorType2>
    constexpr decltype(auto)
    operator/=(const VectorType2<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] /= rhs[i];

        return this_vec();
    }
    constexpr decltype(auto)
    operator/=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) this_vec()[i] /= rhs;

        return this_vec();
    }
};

template <std::size_t idx, typename OutType, std::size_t OutSize, typename InType, typename... Tail>
constexpr std::array<OutType, OutSize>&
vector_constructor(std::array<OutType, OutSize>& out, const InType& head, Tail&&... tail) noexcept
{
    static_assert(idx < OutSize, "Too many elements.");

    out[idx] = static_cast<OutType>(head);

    if constexpr(sizeof...(tail) > 0)
        return vector_constructor<idx + 1>(out, std::forward<Tail>(tail)...);

    if constexpr(idx < (OutSize - 1)) return vector_constructor<idx + 1>(out, head);

    if constexpr(sizeof...(tail) == 0 && (idx + 1) < OutSize)
        return vector_constructor<idx + 1>(out, OutType{0});

    return out;
}

template <std::size_t idx,
          typename OutType,
          std::size_t OutSize,
          typename InType,
          std::size_t InSize,
          template <typename, std::size_t>
          typename Container,
          template <typename, std::size_t, template <typename, std::size_t> typename>
          typename VectorType,
          typename... Tail>
constexpr std::array<OutType, OutSize>&
vector_constructor(std::array<OutType, OutSize>&                out,
                   const VectorType<InType, InSize, Container>& head,
                   Tail&&... tail) noexcept
{
    static_assert(idx < OutSize, "Too many elements.");

    const constexpr auto N = std::min(OutSize - idx, InSize);
    for(std::size_t i = 0; i < N; ++i) out[idx + i] = static_cast<OutType>(head[i]);

    if constexpr(sizeof...(tail) > 0)
        return vector_constructor<idx + N>(out, std::forward<Tail>(tail)...);

    // fill remaining with zeros
    if constexpr(sizeof...(tail) == 0 && (idx + N) < OutSize)
        return vector_constructor<idx + N>(out, OutType{0});

    return out;
}
} // namespace lucid
