// -*- C++ -*-
// matrix.hpp --
//

/// @file
/// Definition of Matrix.

#pragma once

#include "vector.hpp"

namespace lucid
{
/// @brief Generic row-major two-dimensional Matrix of constant size.
/// @tparam T value type. Must be arithmetic.
/// @tparam M number of rows.
/// @tparam N number of columns.
/// @tparam Container std::array-like container that used to store the data.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
class Matrix
{
    static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");
    static_assert(M > 1 && N > 1, "One-dimensional matrices are not supported.");

    static const constexpr std::size_t MN = M * N;

    using Data = Container<T, MN>;
    Data m_data{};

    static constexpr auto
    pos(const std::size_t i, const std::size_t j, const std::size_t J = N) noexcept
    {
        return i * J + j;
    }

    static constexpr auto
    indices(const std::size_t pos) noexcept
    {
        return std::pair(pos / N, pos % N);
    }

    // variyng template sink
    template <std::size_t>
    constexpr void
    unpack() const noexcept
    {
    }

    template <std::size_t idx, typename... Ts>
    constexpr void
    unpack(const T& head, Ts&&... tail) noexcept
    {
        static_assert(idx < MN, "Too many elements.");

        m_data[idx] = head;

        if constexpr(sizeof...(tail) == 0 && idx < MN - 1)
            unpack<idx + 1>(head);
        else
            unpack<idx + 1>(std::forward<Ts>(tail)...);
    }

    // unpack vector arguments
    template <std::size_t idx,
              std::size_t MN1,
              template <typename, std::size_t>
              typename Container2,
              typename... Ts>
    constexpr void
    unpack(const Vector<T, MN1, Container2>& head, Ts&&... tail) noexcept
    {
        static_assert(idx < MN, "Too many elements.");

        const constexpr auto end = std::min(MN - idx, MN1);

        for(std::size_t i = 0; i < end; ++i) m_data[idx + i] = head[i];

        unpack<idx + end>(std::forward<Ts>(tail)...);
    }

    template <std::size_t idx, std::size_t MN1, typename... Ts>
    constexpr void
    unpack(const Container<T, MN1>& head, Ts&&... tail) noexcept
    {
        static_assert(idx < MN, "Too many elements.");
        const constexpr auto end = std::min(MN - idx, MN1);
        for(std::size_t i = 0; i < end; ++i) m_data[idx + i] = head[i];
        unpack<idx + end>(std::forward<Ts>(tail)...);
    }

    template <std::size_t idx,
              std::size_t M1,
              std::size_t N1,
              template <typename, std::size_t>
              typename Container1,
              typename... Ts>
    constexpr void
    unpack(const Matrix<T, M1, N1, Container1>& head, Ts&&... tail) noexcept
    {
        const constexpr auto shift   = indices(idx);
        const constexpr auto shift_i = shift.first;
        const constexpr auto shift_j = shift.second;
        const constexpr auto I       = std::min(M - shift_i, M1);
        const constexpr auto J       = std::min(N - shift_j, N1);

        for(std::size_t i = 0; i < I; ++i)
            for(std::size_t j = 0; j < J; ++j)
                m_data[pos(shift_i + i, shift_j + j)] = head.at(i, j);

        unpack<pos(I, J, J) + 1>(std::forward<Ts>(tail)...);
    }

  public:
    constexpr Matrix() noexcept {}

    constexpr Matrix(const Matrix& rhs) noexcept : m_data(rhs.m_data) {}

    constexpr Matrix(Matrix&& rhs) noexcept : m_data(std::move(rhs.m_data)) {}

    // single scalar construcor
    explicit constexpr Matrix(T&& rhs) noexcept
    {
        for(auto& elem: m_data) elem = rhs;
    }

    explicit constexpr Matrix(Data&& rhs) noexcept : m_data(std::move(rhs)) {}

    explicit constexpr Matrix(Data& rhs) noexcept : m_data(rhs) {}

    explicit constexpr Matrix(const Data& rhs) noexcept : m_data(rhs) {}

    template <typename... Ts>
    explicit constexpr Matrix(Ts&&... rhs) noexcept
    {
        unpack<0>(std::forward<Ts>(rhs)...);
    }

    constexpr Matrix&
    operator=(const Matrix& rhs) noexcept
    {
        m_data = rhs.m_data;
        return *this;
    }

    constexpr Matrix&
    operator=(Matrix&& rhs) noexcept
    {
        m_data = std::move(rhs.m_data);
        return *this;
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Matrix&
    operator=(const Matrix<T, M, N, Container2>& rhs) noexcept
    {
        std::copy(rhs.cbegin(), rhs.cend(), begin());
        return *this;
    }

    constexpr Matrix&
    operator=(const T& rhs) noexcept
    {
        for(T& i: m_data) i = rhs;
        return *this;
    }

    constexpr decltype(auto)
    operator[](const std::size_t i) const noexcept
    {
        CHECK_INDEX(i, M);
        return Vector(StaticSpan<T, N>(at(i, 0)));
    }
    constexpr auto
    operator[](const std::size_t i) noexcept
    {
        CHECK_INDEX(i, M);
        return Vector(StaticSpan<T, N>(at(i, 0)));
    }

    template <std::size_t I>
    constexpr auto
    get() const noexcept
    {
        return Vector(StaticSpan<T, N>(std::get<pos(I, 0)>(m_data)));
    }

    template <std::size_t I>
    constexpr auto
    get() noexcept
    {
        return Vector(StaticSpan<T, N>(std::get<pos(I, 0)>(m_data)));
    }

    template <std::size_t I, std::size_t J>
    constexpr const T&
    get() const noexcept
    {
        static_assert(I < M || J < N, "Index out of range");
        return std::get<pos(I, J)>(m_data);
    }

    template <std::size_t I, std::size_t J>
    constexpr T&
    get() noexcept
    {
        static_assert(I < M || J < N, "Index out of range");
        return std::get<pos(I, J)>(m_data);
    }

    const constexpr T&
    at(const std::size_t i) const noexcept
    {
        CHECK_INDEX(i, MN);
        return m_data[i];
    }
    constexpr T&
    at(const std::size_t i) noexcept
    {
        CHECK_INDEX(i, MN);
        return m_data[i];
    }

    const constexpr T&
    at(const std::size_t i, const std::size_t j) const noexcept
    {
        CHECK_INDEX(i, M);
        CHECK_INDEX(j, N);
        return m_data[pos(i, j)];
    }
    constexpr T&
    at(const std::size_t i, const std::size_t j) noexcept
    {
        CHECK_INDEX(i, M);
        CHECK_INDEX(j, N);
        return m_data[pos(i, j)];
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
    constexpr decltype(auto)
    cbegin() const noexcept
    {
        return m_data.cbegin();
    }
    constexpr decltype(auto)
    cend() const noexcept
    {
        return m_data.cend();
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Matrix&
    operator+=(const Matrix<T, M, N, Container2>& rhs)
    {
        for(std::size_t i = 0; i < MN; ++i) at(i) += rhs.at(i);

        return *this;
    }
    constexpr Matrix&
    operator+=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < MN; ++i) at(i) += rhs;

        return *this;
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Matrix&
    operator-=(const Matrix<T, M, N, Container2>& rhs)
    {
        for(std::size_t i = 0; i < MN; ++i) at(i) -= rhs.at(i);

        return *this;
    }
    constexpr Matrix&
    operator-=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < MN; ++i) at(i) -= rhs;

        return *this;
    }

    template <template <typename, std::size_t> typename VContainer>
    constexpr Vector<T, N, std::array>
    operator*(const Vector<T, N, VContainer>& vec) const noexcept
    {
        return dot(*this, vec);
    }
    template <std::size_t M2, std::size_t N2, template <typename, std::size_t> typename Container2>
    constexpr typename std::enable_if_t<N == M2, Matrix<T, M, N2, std::array>>
    operator*(const Matrix<T, M2, N2, Container2>& rhs) const noexcept
    {
        return dot(*this, rhs);
    }
    constexpr Matrix
    operator*(const T& rhs) const noexcept
    {
        Matrix ret{};

        for(std::size_t i = 0; i < MN; ++i) ret.at(i) = at(i) * rhs;

        return ret;
    }
    constexpr Matrix&
    operator*=(const T& rhs) noexcept
    {
        for(std::size_t i = 0; i < MN; ++i) at(i) *= rhs;

        return *this;
    }

    const constexpr Data&
    data() const noexcept
    {
        return m_data;
    }

    static constexpr Matrix
    identity()
    {
        Matrix ret{};

        for(std::size_t i = 0; i < M; ++i)
            for(std::size_t j = 0; j < N; ++j) ret.at(i, j) = T(i == j);

        return ret;
    }
};

namespace detail
{
template <std::size_t I,
          std::size_t J,
          std::size_t Idx,
          std::size_t Jdx,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr T
minor_matrix_op(const Matrix<T, M, N, Container>& m) noexcept
{
    constexpr std::size_t idx = Idx >= I ? Idx + 1 : Idx;
    constexpr std::size_t jdx = Jdx >= J ? Jdx + 1 : Jdx;
    return m.template get<idx, jdx>();
}

template <std::size_t I,
          std::size_t J,
          std::size_t Idx,
          std::size_t... Jdxs,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
minor_matrix_jdxs(const Matrix<T, M, N, Container>& m, std::index_sequence<Jdxs...>) noexcept
{
    return std::tuple{minor_matrix_op<I, J, Idx, Jdxs>(m)...};
}

template <std::size_t I,
          std::size_t J,
          std::size_t... Idxs,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
minor_matrix_impl(const Matrix<T, M, N, Container>& m, std::index_sequence<Idxs...>) noexcept
{
    return std::tuple_cat(minor_matrix_jdxs<I, J, Idxs>(m, std::make_index_sequence<N - 1>{})...);
}
} // namespace detail

/// @brief Create reference object of input matrix, i.e. matrix view.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr Matrix<T, M, N, StaticSpan>
ref(const Matrix<T, M, N, Container>& m) noexcept
{
    return Matrix<T, M, N, StaticSpan>(StaticSpan<T, M * N>(m.at(0)));
}

/// @brief Create vector view of size @f$M N@f$ of input matrix.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr Vector<T, M * N, StaticSpan>
flat_ref(const Matrix<T, M, N, Container>& m) noexcept
{
    return Vector(StaticSpan<T, M * N>(m.at(0)));
}

namespace fn
{
/// @brief Perform matrix transposition.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
transpose(const Matrix<T, M, N, Container>& a) noexcept
{
    Matrix<T, N, M, std::array> ret{};

    for(std::size_t i = 0; i < N; ++i)
        for(std::size_t j = 0; j < M; ++j) ret[i][j] = a[j][i];

    return ret;
}

/// @brief Compute product of matrix and column-vector.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename MContainer,
          template <typename, std::size_t>
          typename VContainer>
constexpr Vector<T, N, std::array>
dot(const Matrix<T, M, N, MContainer>& lhs, const Vector<T, N, VContainer>& rhs) noexcept
{
    Vector<T, N, std::array> ret{};

    for(std::size_t i = 0; i < M; ++i)
        for(std::size_t j = 0; j < N; ++j) ret[i] += lhs.at(i, j) * rhs[j];

    return ret;
}

/// @brief Compute matrix product.
template <typename T,
          std::size_t M1,
          std::size_t N1,
          std::size_t M2,
          std::size_t N2,
          template <typename, std::size_t>
          typename Container1,
          template <typename, std::size_t>
          typename Container2>
constexpr typename std::enable_if_t<N1 == M2, Matrix<T, M1, N2, std::array>>
dot(const Matrix<T, M1, N1, Container1>& lhs, const Matrix<T, M2, N2, Container2>& rhs) noexcept
{
    Matrix<T, M1, N2, std::array> ret{};

    for(std::size_t i = 0; i < M1; ++i)
        for(std::size_t j = 0; j < N2; ++j)
            for(std::size_t r = 0; r < N1; ++r) ret.at(i, j) += lhs.at(i, r) * rhs.at(r, j);

    return ret;
}

/// @brief Compite matrix determinant.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr typename std::enable_if_t<M == N, T>
det(const Matrix<T, M, N, Container>& a) noexcept
{
    std::array<std::size_t, M> idxs{};
    std::iota(idxs.begin(), idxs.end(), 0);

    auto product = [&idxs, &a]() noexcept {
        T ret{1};
        for(std::size_t i = 0; i < M; ++i) ret *= a.at(i, idxs[i]);
        return ret;
    };

    auto get_elem = [&]() noexcept { return sgn(idxs) * product(); };

    T ret = get_elem();

    const constexpr std::size_t rank = fac(M) - 1;
    for(std::size_t _ = 0; _ < rank; ++_)
    {
        std::next_permutation(idxs.begin(), idxs.end());
        ret += get_elem();
    }

    return ret;
}

/// @brief Contructs minor matrix by removing I row, J column.
template <std::size_t I,
          std::size_t J,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr Matrix<T, M - 1, N - 1, std::array>
minor_matrix(const Matrix<T, M, N, Container>& m) noexcept
{
    static_assert(I < M || J < N, "Indicies out of range");
    return std::make_from_tuple<Matrix<T, M - 1, N - 1, std::array>>(
        detail::minor_matrix_impl<I, J>(m, std::make_index_sequence<M - 1>{}));
}

namespace detail
{
template <std::size_t I,
          std::size_t J,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr T
cofactor_op(const Matrix<T, M, N, Container>& m) noexcept
{
    return minus_one_pow(I + J) * det(minor_matrix<I, J>(m));
}

template <std::size_t I,
          std::size_t... Js,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
cofactor_js(const Matrix<T, M, N, Container>& m, std::index_sequence<Js...>) noexcept
{
    return std::tuple{cofactor_op<I, Js>(m)...};
}

template <std::size_t... Is,
          typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
cofactor_impl(const Matrix<T, M, N, Container>& m, std::index_sequence<Is...>) noexcept
{
    return std::tuple_cat(cofactor_js<Is>(m, std::make_index_sequence<N>{})...);
}
} // namespace detail

/// @brief Compute cofactor matrix.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr Matrix<T, M, N, std::array>
cofactor(const Matrix<T, M, N, Container>& m) noexcept
{
    return std::make_from_tuple<Matrix<T, M, N, std::array>>(
        detail::cofactor_impl(m, std::make_index_sequence<M>{}));
}

/// @brief Compute inverse matrix.
template <typename T,
          std::size_t M,
          std::size_t N,
          template <typename, std::size_t>
          typename Container>
constexpr auto
inverse(const Matrix<T, M, N, Container>& a) noexcept
{
    const auto is_zero = [](const T& val) noexcept {
        if constexpr(std::is_floating_point_v<T>)
            return almost_equal(val, T{0}, 5);
        else
            return val == 0;
    };

    const auto d = det(a);

    if(!is_zero(d))
        return transpose(cofactor(a)) * (T{1} / d);
    else
        return a;
}
} // namespace fn

MK_FN_OBJ(transpose)
MK_FN_OBJ(det)
MK_FN_OBJ(inverse)
} // namespace lucid

namespace std
{
template <typename T, size_t M, size_t N, template <typename, size_t> typename Container>
class tuple_size<lucid::Matrix<T, M, N, Container>>
{
  public:
    static const constexpr size_t value = M;
};

template <size_t I, typename T, size_t M, size_t N, template <typename, size_t> typename Container>
class tuple_element<I, lucid::Matrix<T, M, N, Container>>
{
  public:
    using type = decltype(declval<lucid::Matrix<T, M, N, Container>>().template get<I>());
};

template <auto I, typename T, size_t M, size_t N, template <typename, size_t> typename Container>
constexpr decltype(auto)
get(const lucid::Matrix<T, M, N, Container>& mat) noexcept
{
    return mat.template get<I>();
}
} // namespace std
