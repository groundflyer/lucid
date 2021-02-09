// -*- C++ -*-
// vector.hpp --
//

/// @file
/// Definition of generic N-dimenstional Vector and its functions.

#pragma once

#include <utils/debug.hpp>
#include <utils/functional.hpp>
#include <utils/math.hpp>
#include <utils/static_span.hpp>

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

// Generates code for arithmetic binary operators both for Vector and Matrix.
#define MAKE_BINARY_OP(FUNC, OP)                                                           \
    namespace fn                                                                           \
    {                                                                                      \
    struct FUNC##_fn                                                                       \
    {                                                                                      \
        template <typename T1,                                                             \
                  std::size_t N,                                                           \
                  template <typename, std::size_t>                                         \
                  typename Container,                                                      \
                  typename T2>                                                             \
        constexpr auto                                                                     \
        operator()(const Vector<T1, N, Container>& lhs, const T2& rhs) const noexcept      \
        {                                                                                  \
            return transform(std::FUNC<T1>{}, lhs, rhs);                                   \
        }                                                                                  \
        template <typename T,                                                              \
                  std::size_t M,                                                           \
                  std::size_t N,                                                           \
                  template <typename, std::size_t>                                         \
                  typename Container1,                                                     \
                  template <typename, std::size_t>                                         \
                  typename Container2>                                                     \
        constexpr Matrix<T, M, N, std::array>                                              \
        operator()(const Matrix<T, M, N, Container1>& lhs,                                 \
                   const Matrix<T, M, N, Container2>& rhs) const noexcept                  \
        {                                                                                  \
            constexpr std::FUNC<T>      op{};                                              \
            Matrix<T, M, N, std::array> ret{};                                             \
            for(std::size_t i = 0; i < (M * N); ++i) ret.at(i) = op(lhs.at(i), rhs.at(i)); \
            return ret;                                                                    \
        }                                                                                  \
        template <typename T,                                                              \
                  std::size_t M,                                                           \
                  std::size_t N,                                                           \
                  template <typename, std::size_t>                                         \
                  typename Container>                                                      \
        constexpr Matrix<T, M, N, std::array>                                              \
        operator()(const Matrix<T, M, N, Container>& lhs, const T& rhs) const noexcept     \
        {                                                                                  \
            constexpr std::FUNC<T>      op{};                                              \
            Matrix<T, M, N, std::array> ret{};                                             \
            for(std::size_t i = 0; i < (M * N); ++i) ret.at(i) = op(lhs.at(i), rhs);       \
            return ret;                                                                    \
        }                                                                                  \
        template <typename Rhs>                                                            \
        constexpr decltype(auto)                                                           \
        operator^(const Rhs& rhs) const noexcept                                           \
        {                                                                                  \
            return compose(*this, rhs);                                                    \
        }                                                                                  \
    };                                                                                     \
    }                                                                                      \
    static constexpr fn::FUNC##_fn FUNC{};                                                 \
    template <typename T1,                                                                 \
              std::size_t N,                                                               \
              template <typename, std::size_t>                                             \
              typename Container,                                                          \
              typename T2>                                                                 \
    constexpr auto OP(const Vector<T1, N, Container>& lhs, const T2& rhs) noexcept         \
    {                                                                                      \
        return FUNC(lhs, rhs);                                                             \
    }                                                                                      \
    template <typename T1,                                                                 \
              std::size_t M,                                                               \
              std::size_t N,                                                               \
              template <typename, std::size_t>                                             \
              typename Container,                                                          \
              typename T2>                                                                 \
    constexpr auto OP(const Matrix<T1, M, N, Container>& lhs, const T2& rhs) noexcept      \
    {                                                                                      \
        return FUNC(lhs, rhs);                                                             \
    }

#define MAKE_UNARY_OP(FUNC, OP)                                                                   \
    namespace fn                                                                                  \
    {                                                                                             \
    struct FUNC##_fn                                                                              \
    {                                                                                             \
        template <typename T, std::size_t N, template <typename, std::size_t> typename Container> \
        constexpr Vector<T, N, std::array>                                                        \
        operator()(const Vector<T, N, Container>& lhs) const noexcept                             \
        {                                                                                         \
            return transform(std::FUNC<T>(), lhs);                                                \
        }                                                                                         \
        template <typename T,                                                                     \
                  std::size_t M,                                                                  \
                  std::size_t N,                                                                  \
                  template <typename, std::size_t>                                                \
                  typename Container>                                                             \
        constexpr Matrix<T, M, N, std::array>                                                     \
        operator()(const Matrix<T, M, N, Container>& lhs) const noexcept                          \
        {                                                                                         \
            return Matrix<T, M, N, std::array>(transform(std::FUNC<T>(), flat_ref(lhs)));         \
        }                                                                                         \
        template <typename Rhs>                                                                   \
        constexpr decltype(auto)                                                                  \
        operator^(const Rhs& rhs) const noexcept                                                  \
        {                                                                                         \
            return compose(*this, rhs);                                                           \
        }                                                                                         \
    };                                                                                            \
    }                                                                                             \
    static constexpr fn::FUNC##_fn FUNC{};                                                        \
    template <typename T, std::size_t N, template <typename, std::size_t> typename Container>     \
    constexpr Vector<T, N, std::array> OP(const Vector<T, N, Container>& lhs) noexcept            \
    {                                                                                             \
        return FUNC(lhs);                                                                         \
    }                                                                                             \
    template <typename T,                                                                         \
              std::size_t M,                                                                      \
              std::size_t N,                                                                      \
              template <typename, std::size_t>                                                    \
              typename Container>                                                                 \
    constexpr Matrix<T, M, N, std::array> OP(const Matrix<T, M, N, Container>& lhs) noexcept      \
    {                                                                                             \
        return FUNC(lhs);                                                                         \
    }

#define VEC_NAMED_FN_OBJ(NAME, FUNC)                                                     \
    namespace fn                                                                         \
    {                                                                                    \
    struct NAME##_fn                                                                     \
    {                                                                                    \
        template <typename T,                                                            \
                  std::size_t N,                                                         \
                  template <typename, std::size_t>                                       \
                  typename Container,                                                    \
                  typename... Args>                                                      \
        constexpr decltype(auto)                                                         \
        operator()(const Vector<T, N, Container>& v, const Args&... args) const noexcept \
        {                                                                                \
            return transform(*this, v, args...);                                         \
        }                                                                                \
        template <typename... Args>                                                      \
        constexpr decltype(auto)                                                         \
        operator()(const Args&... args) const noexcept                                   \
        {                                                                                \
            return FUNC(args...);                                                        \
        }                                                                                \
        template <typename Rhs>                                                          \
        constexpr decltype(auto)                                                         \
        operator^(const Rhs& rhs) const noexcept                                         \
        {                                                                                \
            return compose(*this, rhs);                                                  \
        }                                                                                \
    };                                                                                   \
    }                                                                                    \
    static constexpr fn::NAME##_fn NAME{};

#define VEC_FN_OBJ(FUNC) VEC_NAMED_FN_OBJ(FUNC, FUNC)

#define VEC_NAMED_FN_OBJ_SMATH(FUNC)                                                     \
    namespace fn                                                                         \
    {                                                                                    \
    struct FUNC##_fn                                                                     \
    {                                                                                    \
        template <typename T,                                                            \
                  std::size_t N,                                                         \
                  template <typename, std::size_t>                                       \
                  typename Container,                                                    \
                  typename... Args>                                                      \
        constexpr decltype(auto)                                                         \
        operator()(const Vector<T, N, Container>& v, const Args&... args) const noexcept \
        {                                                                                \
            return transform(*this, v, args...);                                         \
        }                                                                                \
        template <typename... Args>                                                      \
        constexpr decltype(auto)                                                         \
        operator()(const Args&... args) const noexcept                                   \
        {                                                                                \
            if(std::is_constant_evaluated())                                             \
                return smath::FUNC(args...);                                             \
            else                                                                         \
                return std::FUNC(args...);                                               \
        }                                                                                \
        template <typename Rhs>                                                          \
        constexpr decltype(auto)                                                         \
        operator^(const Rhs& rhs) const noexcept                                         \
        {                                                                                \
            return compose(*this, rhs);                                                  \
        }                                                                                \
    };                                                                                   \
    }                                                                                    \
    static constexpr fn::FUNC##_fn FUNC{};

namespace lucid
{
template <typename T, size_t M, size_t N, template <typename, size_t> typename Container>
class Matrix;

/// @brief Generic N-dimensional euclidian vector.
/// @tparam T value type. Must be arithmetic.
/// @tparam N dimensionality.
/// @tparam Container std::array-like container that used to store the data.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
class Vector
{
    static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

    using Data = Container<T, N>;

    Data m_data{};

    template <std::size_t idx>
    constexpr void
    unpack()
    {
        if constexpr(idx < N)
            for(auto i = idx; i < N; ++i) m_data[i] = static_cast<T>(0);
    }

    template <std::size_t idx, typename... Ts>
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

    template <std::size_t idx,
              typename T1,
              std::size_t N1,
              template <typename, std::size_t>
              typename Container1,
              typename... Ts>
    constexpr void
    unpack(const Vector<T1, N1, Container1>& first, const Ts&... other)
    {
        static_assert(idx < N, "Too many elements.");
        for(std::size_t i = 0; i < std::min(N - idx, N1); ++i)
            m_data[idx + i] = static_cast<T>(first[i]);
        unpack<idx + N1>(other...);
    }

    template <std::size_t idx, typename T1, std::size_t N1, typename... Ts>
    constexpr void
    unpack(const Container<T1, N1>& first, const Ts&... other)
    {
        static_assert(idx < N, "Too many elements.");
        for(std::size_t i = 0; i < std::min(N - idx, N1); ++i)
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
    template <template <typename, std::size_t> typename Container2>
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

    constexpr const T&
    at(const std::size_t i) const noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }
    constexpr T&
    at(const std::size_t i) noexcept
    {
        CHECK_INDEX(i, N);
        return m_data[i];
    }

    constexpr const Data&
    data() const noexcept
    {
        return m_data;
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Vector&
    operator+=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] += rhs[i];

        return *this;
    }
    constexpr Vector&
    operator+=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] += rhs;

        return *this;
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Vector&
    operator-=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] -= rhs[i];

        return *this;
    }
    constexpr Vector&
    operator-=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] -= rhs;

        return *this;
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Vector&
    operator*=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] *= rhs[i];

        return *this;
    }
    constexpr Vector&
    operator*=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] *= rhs;

        return *this;
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Vector&
    operator/=(const Vector<T, N, Container2>& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] /= rhs[i];

        return *this;
    }

    constexpr Vector&
    operator/=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < N; ++i) m_data[i] /= rhs;

        return *this;
    }

    constexpr std::size_t
    size() const noexcept
    {
        return N;
    }
};

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
Vector(Container<T, N> &&) -> Vector<T, N, Container>;

namespace detail
{
template <typename T>
struct vector_val_type
{
    using type = T;
};

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
struct vector_val_type<Vector<T, N, Container>>
{
    using type = T;
};

template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr const T&
elem(const Vector<T, N, Container>& v, const std::size_t i) noexcept
{
    return v[i];
}

template <typename T>
constexpr const T&
elem(const T& v, const std::size_t) noexcept
{
    return v;
}

template <std::size_t I, typename... Args>
constexpr decltype(auto)
elem(const std::size_t i, const Args&... vectors) noexcept
{
    return elem(select<I>(vectors...), i);
}

template <typename... Vectors>
struct vector_dim;

template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          typename... Rest>
struct vector_dim<Vector<T, N, Container>, Rest...>
{
    static constexpr std::size_t n = N;
};

template <std::size_t N, typename T>
struct check_dim
{
    static constexpr bool value = true;
};

template <std::size_t N1,
          typename T,
          std::size_t N2,
          template <typename, std::size_t>
          typename Container>
struct check_dim<N1, Vector<T, N2, Container>>
{
    static constexpr bool value = N1 == N2;
};

template <typename T>
struct promote_tuple
{
    template <std::size_t N>
    using ret_type = Vector<std::decay_t<T>, N, std::array>;

    template <std::size_t N>
    static constexpr void
    assign(ret_type<N>& v, const std::size_t i, const T& val) noexcept
    {
        v[i] = val;
    }
};

template <typename T1, typename T2>
struct promote_tuple<std::pair<T1, T2>>
{
    template <std::size_t N>
    using ret_type =
        std::pair<Vector<std::decay_t<T1>, N, std::array>, Vector<std::decay_t<T2>, N, std::array>>;

    template <std::size_t N>
    static constexpr void
    assign(ret_type<N>& v, const std::size_t i, const std::pair<T1, T2>& val) noexcept
    {
        std::tie(v.first[i], v.second[i]) = val;
    }
};

template <typename... Ts>
struct promote_tuple<std::tuple<Ts...>>
{
    template <std::size_t N>
    using ret_type = std::tuple<Vector<std::decay_t<Ts>, N, std::array>...>;

    template <std::size_t N>
    static constexpr void
    assign(ret_type<N>& v, const std::size_t i, const std::tuple<Ts...>& val) noexcept
    {
        std::apply([&val, &i](Ts&... vv) noexcept { std::tie(vv[i]...) = val; }, v);
    }
};

template <typename F, std::size_t... Idxs, typename... Vectors>
constexpr auto
transform_impl(const F& f, std::index_sequence<Idxs...>, const Vectors&... vs) noexcept
{
    using val_type = std::invoke_result_t<
        F,
        typename vector_val_type<std::tuple_element_t<Idxs, std::tuple<Vectors...>>>::type...>;
    constexpr std::size_t N = vector_dim<Vectors...>::n;
    static_assert((true && ... && check_dim<N, Vectors>::value),
                  "Vectors should have the same dimensionality");
    using pt       = promote_tuple<val_type>;
    using ret_type = typename pt::template ret_type<N>;

    ret_type ret{};

    for(std::size_t i = 0; i < N; ++i) pt::assign(ret, i, f(elem<Idxs>(i, vs...)...));

    return ret;
}

template <int         _Sgn,
          std::size_t J,
          std::size_t K,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr T
cross_impl_factor_ab(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return _Sgn * a.template get<J>() * b.template get<K>();
}

template <std::size_t I,
          std::size_t J,
          std::size_t K,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr T
cross_impl_factor(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return cross_impl_factor_ab<sgn(std::array<std::size_t, 3>{I, J, K}), J, K>(a, b);
}

template <std::size_t I,
          std::size_t J,
          std::size_t... Ks,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr T
cross_impl_ks(const Vector<T, N, Container1>& a,
              const Vector<T, N, Container2>& b,
              std::index_sequence<Ks...>) noexcept
{
    return (T{0} + ... + cross_impl_factor<I, J, Ks>(a, b));
}

template <std::size_t I,
          std::size_t... Js,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr T
cross_impl_js(const Vector<T, N, Container1>& a,
              const Vector<T, N, Container2>& b,
              std::index_sequence<Js...>) noexcept
{
    return (T{0} + ... + cross_impl_ks<I, Js>(a, b, std::make_index_sequence<N>{}));
}

template <std::size_t... Is,
          typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr Vector<T, N, std::array>
cross_impl(const Vector<T, N, Container1>& a,
           const Vector<T, N, Container2>& b,
           std::index_sequence<Is...>) noexcept
{
    return Vector<T, N, std::array>(cross_impl_js<Is>(a, b, std::make_index_sequence<N>{})...);
}
} // namespace detail

/// @brief Create reference to input vector, i.e. vector view.
///
/// Creates a such vector object that data it contains is mapped to
/// other place.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr const Vector<T, N, StaticSpan>
ref(const Vector<T, N, Container>& v) noexcept
{
    return Vector<T, N, StaticSpan>(StaticSpan<T, N>(v.template get<0>()));
}

template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr Vector<T, M * N, StaticSpan>
flat_ref(const Matrix<T, M, N, Container>& m) noexcept;

/// @brief Apply function to corresponding elements of input vectors.
/// @return Vector containing results of applycation @f to the elements of input vectors
/// or pair or tuple of vectors if @f returns such.
/// @p f accepts as many arguments as the number of input vectors @p vs.
/// @p Vectors should have the same dimensionality.
template <typename F, typename... Vectors>
constexpr auto
transform(const F& f, const Vectors&... vs) noexcept
{
    return detail::transform_impl(f, std::index_sequence_for<Vectors...>{}, vs...);
}

/// @brief Perform left fold on a given vector.
/// @param binary_op folding operator.
/// @param a vector to fold.
/// @param init initial value.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          typename BinaryOperation,
          typename Init>
constexpr auto
reduce(BinaryOperation binary_op, const Vector<T, N, Container>& a, Init init) noexcept
{
    for(std::size_t i = 0; i < N; ++i) init = binary_op(init, a[i]);

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
    for(std::size_t i = 0; i < N; ++i) init = binary_op2(init, binary_op1(a[i], b[i]));

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
          typename UnaryOperation,
          typename BinaryOperation,
          typename Init>
constexpr auto
transform_reduce(UnaryOperation                 unary_op,
                 BinaryOperation                binary_op,
                 const Vector<T, N, Container>& a,
                 Init                           init) noexcept
{
    for(std::size_t i = 0; i < N; ++i) init = binary_op(init, unary_op(a[i]));

    return init;
}

VEC_FN_OBJ(almost_equal)
VEC_FN_OBJ(lerp)
VEC_NAMED_FN_OBJ(isfinite, std::isfinite)
VEC_NAMED_FN_OBJ(clamp, std::clamp)
VEC_NAMED_FN_OBJ(minmax, std::minmax)
VEC_NAMED_FN_OBJ(fmod, std::fmod)
VEC_NAMED_FN_OBJ(atan, std::atan)
// functions that have static_math counterpart
VEC_NAMED_FN_OBJ_SMATH(sqrt)
VEC_NAMED_FN_OBJ_SMATH(abs)
VEC_NAMED_FN_OBJ_SMATH(pow)
VEC_NAMED_FN_OBJ_SMATH(sin)
VEC_NAMED_FN_OBJ_SMATH(cos)
VEC_NAMED_FN_OBJ_SMATH(tan)
VEC_NAMED_FN_OBJ_SMATH(round)
VEC_NAMED_FN_OBJ_SMATH(floor)

namespace fn
{
/// we define multiplies and operator* separately because we don't want to override Matrix
/// multiplication
struct multiplies_fn
{
    template <typename T1,
              std::size_t N,
              template <typename, std::size_t>
              typename Container,
              typename T2>
    constexpr auto
    operator()(const Vector<T1, N, Container>& lhs, const T2& rhs) const noexcept
    {
        return transform(std::multiplies<T1>(), lhs, rhs);
    }
    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};

/// @brief Compute dot product of the given vectors.
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr T
dot(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return transform_reduce(std::multiplies<T>(), std::plus<T>(), a, b, T{0});
}

template <typename T,
          size_t M,
          size_t N,
          template <typename, size_t>
          typename MContainer,
          template <typename, size_t>
          typename VContainer>
constexpr Vector<T, N, std::array>
dot(const Matrix<T, M, N, MContainer>& lhs, const Vector<T, N, VContainer>& rhs) noexcept;

template <typename T,
          size_t M1,
          size_t N1,
          size_t M2,
          size_t N2,
          template <typename, size_t>
          typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr typename std::enable_if_t<N1 == M2, Matrix<T, M1, N2, std::array>>
dot(const Matrix<T, M1, N1, Container1>& lhs, const Matrix<T, M2, N2, Container2>& rhs) noexcept;

/// @brief Implements efficient generic N-dimensional cross product.
///
/// Naive implementation would be:
/// @code{.cpp}
/// Vector<T, N, std::array> ret;
/// for(std::size_t i = 0; i < N; ++i)
///     for(std::size_t j = 0; j < N; ++j)
///         for(std::size_t k = 0; k < N; ++k)
///             ret[i] += sgn(std::array<std::size_t, 3>{i, j, k}) * a[j] * b[k];
/// @endcode
template <typename T,
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr auto
cross(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return detail::cross_impl(a, b, std::make_index_sequence<N>{});
}

/// @brief Compute squared length of a vector.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T
length2(const Vector<T, N, Container>& a) noexcept
{
    return transform_reduce(static_pow<2, T>, std::plus<T>(), a, T{0});
}

/// @brief Compute length of a vector.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T
length(const Vector<T, N, Container>& a) noexcept
{
    return sqrt(length2(a));
}

/// @brief Normalize vector.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
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
          std::size_t N,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr T
distance(const Vector<T, N, Container1>& a, const Vector<T, N, Container2>& b) noexcept
{
    return length(a - b);
}

/// @brief Sum of all vector elements.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T
sum(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::plus<T>(), a, T{0});
}

/// @brief Product of all vector elements.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T
product(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::multiplies<T>(), a, T{1});
}

/// @brief Test whether all vector elements evaluate to true.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr bool
all(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::logical_and<bool>(), a, true);
}

/// @brief Test whether any vector element evaluates to true.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr bool
any(const Vector<T, N, Container>& a) noexcept
{
    return reduce(std::logical_or<bool>(), a, false);
}

/// @brief Compute average of all vector elements.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr T
avg(const Vector<T, N, Container>& a) noexcept
{
    return sum(a) / N;
}

/// @brief Roll vector elements.
/// @param shift the number of places which elements are shifted.
template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
constexpr Vector<T, N, std::array>
roll(const Vector<T, N, Container>& v, const std::size_t shift) noexcept
{
    Vector<T, N, std::array> ret{};

    for(std::size_t i = 0; i < N; ++i) ret[i] = v[(i - shift) % N];

    return ret;
}

/// @brief Rise vector elements to a constant power.
/// @tparam exp power exponent.
template <unsigned exp>
struct static_pow_fn
{
    template <typename T>
    constexpr T
    operator()(const T& value) const noexcept
    {
        return static_pow<exp>(value);
    }

    template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
    constexpr Vector<T, N, Container>
    operator()(const Vector<T, N, Container>& v) const noexcept
    {
        return transform(*this, v);
    }

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};

struct min_fn
{
    template <typename T>
    constexpr decltype(auto)
    operator()(const T& a, const T& b) const noexcept
    {
        if constexpr(std::is_arithmetic_v<T>)
            return std::min(a, b);
        else
            return transform(*this, a, b);
    }

    template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
    constexpr T
    operator()(const Vector<T, N, Container>& a) const noexcept
    {
        return reduce(*this, a, std::numeric_limits<T>::max());
    }

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};

struct max_fn
{
    template <typename T>
    constexpr decltype(auto)
    operator()(const T& a, const T& b) const noexcept
    {
        if constexpr(std::is_arithmetic_v<T>)
            return std::max(a, b);
        else
            return transform(*this, a, b);
    }

    template <typename T, std::size_t N, template <typename, std::size_t> typename Container>
    constexpr T
    operator()(const Vector<T, N, Container>& a) const noexcept
    {
        return reduce(*this, a, std::numeric_limits<T>::min());
    }

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};

template <typename Integer>
struct downsample_fn
{
    static_assert(std::is_integral_v<Integer>);

    template <typename Float>
    constexpr std::enable_if_t<std::is_floating_point_v<Float>, Integer>
    operator()(const Float& value) const noexcept
    {
        Float clamped = std::clamp(value, Float{0}, Float{1});
        clamped *= std::numeric_limits<Integer>::max();
        return static_cast<Integer>(clamped);
    }

    template <typename Float, std::size_t N, template <typename, std::size_t> typename Container>
    constexpr std::enable_if_t<std::is_floating_point_v<Float>, Vector<Integer, N, std::array>>
    operator()(const Vector<Float, N, Container>& vec) const noexcept
    {
        return transform(*this, vec);
    }

    template <typename Rhs>
    constexpr decltype(auto)
    operator^(const Rhs& rhs) const noexcept
    {
        return compose(*this, rhs);
    }
};
} // namespace fn

template <unsigned exp>
static constexpr fn::static_pow_fn<exp> static_pow;
static constexpr fn::min_fn             min;
static constexpr fn::max_fn             max;

/// @brief Downsample floating point value to integer value with maximum integer equal to 1.0 of
/// floating point value.
template <typename Integer>
static constexpr fn::downsample_fn<Integer> downsample;

MK_FN_OBJ(dot)
MK_FN_OBJ(cross)
MK_FN_OBJ(length2)
MK_FN_OBJ(length)
MK_FN_OBJ(normalize)
MK_FN_OBJ(distance)
MK_FN_OBJ(sum)
MK_FN_OBJ(product)
MK_FN_OBJ(all)
MK_FN_OBJ(any)
MK_FN_OBJ(avg)
MK_FN_OBJ(roll)

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

static constexpr fn::multiplies_fn multiplies;

template <typename T1,
          std::size_t N,
          template <typename, std::size_t>
          typename Container,
          typename T2>
constexpr auto
operator*(const Vector<T1, N, Container>& lhs, const T2& rhs) noexcept
{
    return multiplies(lhs, rhs);
}

MAKE_UNARY_OP(negate, operator-)
MAKE_UNARY_OP(logical_not, operator!)
MAKE_BINARY_OP(plus, operator+)
MAKE_BINARY_OP(minus, operator-)
MAKE_BINARY_OP(divides, operator/)
MAKE_BINARY_OP(equal_to, operator==)
MAKE_BINARY_OP(not_equal_to, operator!=)
MAKE_BINARY_OP(greater, operator>)
MAKE_BINARY_OP(less, operator<)
MAKE_BINARY_OP(greater_equal, operator>=)
MAKE_BINARY_OP(less_equal, operator<=)
} // namespace lucid

#undef MAKE_BINARY_OP
#undef MAKE_UNARY_OP
#undef VEC_NAMED_FN_OBJ
#undef VEC_FN_OBJ
#undef VEC_NAMED_FN_OBJ_SMATH

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
