// -*- C++ -*-
/// @file
/// Matrix tests.

#include "property_test.hpp"

#include <base/matrix.hpp>
#include <utils/tuple.hpp>
#include <utils/typeinfo.hpp>
#include <utils/typelist.hpp>

#include <fmt/ranges.h>

#include <random>

using namespace std;
using namespace lucid;

using Indicies        = index_sequence<3ul, 4ul>;
using ArithmeticTypes = type_sequence<float, double>;
// 5% error threshold, because + and - operations are quite inaccurate
static const constexpr double threshold = 0.05;

/// @brief Test lucid::Matrix.
/// @tparam M Number of rows.
/// @tparam N Number of columns.
template <typename T, size_t M, size_t N, typename RandomEngine>
unsigned
matrix_test(RandomEngine& g, const size_t num_tests) noexcept
{
    using Mat                                        = Matrix<T, M, N, array>;
    using Vec                                        = Vector<T, N, array>;
    const constexpr auto              t_typestring   = get_typeinfo_string(T{});
    const auto                        mat_typestring = get_typeinfo_string(Mat{});
    const auto                        vec_typestring = get_typeinfo_string(Vec{});
    const auto                        arr_typestring = get_typeinfo_string(Mat().data());
    std::uniform_real_distribution<T> dist(T{-10000}, T{10000});
    static const constexpr auto       MN = M * N;
    auto       array_mn_gen              = [&]() noexcept { return generate<MN>(dist, g); };
    const auto array_assertion           = [](const Mat& testing, const auto& feed) noexcept {
        bool ret = false;
        for(size_t i = 0; i < MN; ++i) ret |= testing.at(i) != feed[i];
        return ret || !all(lucid::isfinite(flat_ref(testing)));
    };
    auto array_n_gen = [&]() noexcept { return generate<N>(dist, g); };
    auto vgen        = [&]() noexcept { return Vec(array_n_gen()); };

    const auto test_property_n = [num_tests](auto&&... args) noexcept {
        return test_property(num_tests, threshold, forward<decltype(args)>(args)...);
    };

    const auto assertion = [](const auto& a, const auto& b) noexcept {
        if constexpr(is_floating_point_v<T>)
        {
            const constexpr unsigned ULP = 5;
            return any(!almost_equal(flat_ref(a), flat_ref(b), ULP)) ||
                   !all(lucid::isfinite(flat_ref(a))) || !all(lucid::isfinite(flat_ref(b)));
        }
        else
            return any(flat_ref(a) != flat_ref(b));
    };

    auto mat_gen  = [&]() noexcept { return Mat(array_mn_gen()); };
    auto mat_gen1 = [&]() noexcept { return pair{mat_gen(), dist(g)}; };
    auto mat_gen2 = [&]() noexcept { return pair{mat_gen(), mat_gen()}; };

    constexpr auto mmaker = maker<Mat>;
    constexpr auto tmaker = tuple_maker<Mat>;

    unsigned ret = 0u;

    /// @test Construct Matrix with a single scalar.
    ///
    /// @f$\mathrm{M}(a) = \mathrm{M}(a, a, a\dots)@f$
    ret += test_property_n(
        "flat_ref({}({}))"_format(mat_typestring, t_typestring),
        [&]() noexcept { return dist(g); },
        mmaker,
        [](const auto& testing, const auto& feed) noexcept {
            return any(flat_ref(testing) != feed) || !all(lucid::isfinite(flat_ref(testing)));
        });

    /// @test Standard matrix constructor.
    ///
    /// @f$\mathrm{M}(a,b,c\dots) = \mathrm{M}(a, b, c\dots)@f$
    ret += test_property_n(
        "{}({})"_format(mat_typestring, arr_typestring), array_mn_gen, tmaker, array_assertion);

    /// @test Construct Matrix with an array.
    ///
    /// @f$\mathrm{M}([a,b,c\dots]) = \mathrm{M}(a, b, c\dots)@f$
    ret += test_property_n(
        "{}({})"_format(mat_typestring, arr_typestring), array_mn_gen, mmaker, array_assertion);

    /// @test Construct Matrix from few vectors.
    ///
    /// @f$\mathrm{M}_{m,n}(\mathrm{V}_n(a_{1,1},a_{1,2},\dotsc,a_{1,n}),
    /// \mathrm{V}_n(a_{2,1},a_{2,2},\dotsc,a_{2,n}),\dotsc,
    /// \mathrm{V}_n(a_{m,1},a_{m,2},\dotsc,a_{m,n})) =
    /// \begin{pmatrix}
    /// a_{1,1} & a_{1,2} & \cdots & a_{1,n} \\
    /// a_{2,1} & a_{2,2} & \cdots & a_{2,n} \\
    /// \vdots  & \vdots  & \ddots & \vdots  \\
    /// a_{m,1} & a_{m,2} & \cdots & a_{m,n}
    /// \end{pmatrix}@f$
    ret += test_property_n(
        "{}({})"_format(mat_typestring, get_typeinfo_string(array<Vec, M>{})),
        [&]() noexcept {
            array<Vec, M> ret;
            for(size_t i = 0; i < M; ++i) ret[i] = vgen();
            return ret;
        },
        tmaker,
        [](const Mat& testing, const array<Vec, M>& feed) noexcept {
            bool ret = false;
            for(size_t i = 0; i < M; ++i) ret |= any(testing[i] != feed[i]);
            return ret || !all(lucid::isfinite(flat_ref(testing)));
        });

    array<string, M> strs;
    std::fill(strs.begin(), strs.end(), "{}"_format(get_typeinfo_string(Vec().data())));

    /// @test Construct Matrix with few arrays.
    ///
    /// @f$\mathrm{M}_{m,n}([a_{1,1},a_{1,2},\dotsc,a_{1,n}],
    /// [a_{2,1},a_{2,2},\dotsc,a_{2,n}],\dotsc,
    /// [a_{m,1},a_{m,2},\dotsc,a_{m,n}]) =
    /// \begin{pmatrix}
    /// a_{1,1} & a_{1,2} & \cdots & a_{1,n} \\
    /// a_{2,1} & a_{2,2} & \cdots & a_{2,n} \\
    /// \vdots  & \vdots  & \ddots & \vdots  \\
    /// a_{m,1} & a_{m,2} & \cdots & a_{m,n}
    /// \end{pmatrix}@f$
    ret += test_property_n(
        "{}({})"_format(mat_typestring, strs),
        [&]() noexcept {
            array<array<T, N>, M> ret;
            for(size_t i = 0; i < M; ++i) ret[i] = array_n_gen();
            return ret;
        },
        tmaker,
        [](const Mat& testing, const array<array<T, N>, M>& feed) noexcept {
            bool ret = false;
            for(size_t i = 0; i < M; ++i)
                for(size_t j = 0; j < N; ++j) ret |= testing[i][j] != feed[i][j];
            return ret || !all(lucid::isfinite(flat_ref(testing)));
        });

    if constexpr(M > 2 && N > 2)
    {
        const constexpr std::size_t M1  = M - 1;
        const constexpr std::size_t N1  = N - 1;
        const constexpr std::size_t MN1 = M1 * N1;
        using sMat                      = Matrix<T, M1, N1, array>;
        using sVec                      = Vector<T, N1, array>;
        auto smat_gen                   = [&]() noexcept { return sMat(generate<MN1>(dist, g)); };
        auto svecgen                    = [&]() noexcept { return sVec(generate<N1>(dist, g)); };

        /// @test Construct Matrix with a smaller matrix.
        ///
        /// @f$\mathrm{M}_{m,n}\left(
        /// \mathrm{M}_{m-1,n-1}
        /// \begin{pmatrix}
        /// a_{1,1} & \cdots & a_{1,n-1} \\
        /// \vdots & \ddots & \vdots \\
        /// a_{m-1,1} & \cdots & a_{m-1,n-1}
        /// \end{pmatrix} \right) =
        /// \begin{pmatrix}
        /// a_{1,1} & a_{1,2} & \cdots & a_{1,n-1} & \cdots & 0 \\
        /// \vdots  & \vdots  & \vdots & \vdots & \ddots & \vdots \\
        /// a_{m-1,1} & a_{m-1,2} & \cdots & a_{m-1,n-1} & \cdots & 0 \\
        /// \vdots  & \vdots  & \vdots & \vdots & \ddots & \vdots \\
        /// 0 & 0 & \cdots & 0 & \cdots & 0
        /// \end{pmatrix}@f$
        ret +=
            test_property_n("{}(Matrix<{}, {}, {}>)"_format(mat_typestring, t_typestring, M1, N1),
                            smat_gen,
                            mmaker,
                            [](const Mat& testing, const sMat& feed) noexcept {
                                bool ret = false;
                                for(size_t i = 0; i < M1; ++i)
                                    for(size_t j = 0; j < N1; ++j)
                                        ret |= feed.at(i, j) != testing.at(i, j);
                                return ret || !all(lucid::isfinite(flat_ref(testing)));
                            });

        /// @test Construct Matrix from a scalar, a vector and a smaller matrix.
        ///
        /// @f$\mathrm{M}_{m,n}\left(a, \mathrm{V}_{n-1}(b_1,\cdots,b_{n-1}),
        /// \mathrm{M}_{m-1,n-1}
        /// \begin{pmatrix}
        /// c_{1,1} & \cdots & c_{1,n-1} \\
        /// \vdots & \ddots & \vdots \\
        /// c_{m-1,1} & \cdots & c_{k,n-1}
        /// \end{pmatrix} \right) =
        /// \begin{pmatrix}
        /// a & b_1 & \cdots & b_{n-1} \\
        /// c_{1,1} & \cdots & c_{1,n-1} & 0 \\
        /// \vdots  & \ddots & \vdots & \vdots \\
        /// c_{m-1,1} & \cdots & c_{m-1,n-1} & 0
        /// \end{pmatrix}@f$
        ret += test_property_n(
            "{0}({1}, Vector<{1}, {2}>, Matrix<{1}, {3}, {2}>)"_format(
                mat_typestring, t_typestring, N1, M1),
            [&]() noexcept { return tuple(dist(g), svecgen(), smat_gen()); },
            mmaker,
            [](const Mat& testing, const auto& feed) noexcept {
                const auto& [scalar, vec, mat] = feed;
                bool ret                       = testing.at(0) != scalar;
                for(size_t i = 0; i < N1; ++i) ret |= vec[i] != testing[0][i + 1];
                for(size_t i = 0; i < M1; ++i)
                    for(size_t j = 0; j < N1; ++j) ret |= mat.at(i, j) != testing.at(i + 1, j);
                return ret;
            });

        /// @test Matrix inversion.
        ///
        /// @f$\mathbf{A}^T\mathbf{A} = \mathbf{I}_n@f$
        if constexpr(M == N && is_floating_point_v<T>)
            ret += test_property_n(
                "{0}: inverse(A) * A = {0}::identity()"_format(mat_typestring),
                [&]() noexcept { return mat_gen(); },
                inverse,
                [](const Mat& testing, const Mat& feed) noexcept {
                    // identity matrix elements have different magnitude
                    // equalizing them as ulp is same for all of them
                    const Mat              zero = Mat::identity() - feed * testing;
                    const constexpr size_t ulp  = static_pow<sizeof(T)>(100ul);
                    return any(!almost_equal(flat_ref(zero), T{0}, ulp)) ||
                           !all(lucid::isfinite(flat_ref(testing)));
                });
    }

    /// @test Matrix-scalar addition and subtraction.
    ///
    /// @f$\mathbf{A} + b = \mathbf{A} + b - b@f$
    ret += test_property_n("{} +- {}"_format(mat_typestring, t_typestring),
                           mat_gen1,
                           lucid::plus,
                           [&](const Mat& testing, const auto& feed) noexcept {
                               const auto& [mat, scalar] = feed;
                               const auto mm             = testing - scalar;
                               return assertion(mm, mat);
                           });

    /// @test Inplace matrix-scalar addition and subtraction.
    ///
    /// @f$\mathbf{A} + b = \mathbf{A} + b - b@f$
    ret += test_property_n(
        "{} +-= {}"_format(mat_typestring, t_typestring),
        mat_gen1,
        [](Mat mat, const T& scalar) noexcept {
            mat += scalar;
            return mat;
        },
        [&](Mat testing, const auto& feed) noexcept {
            const auto& [mat, scalar] = feed;
            testing -= scalar;
            return assertion(testing, mat);
        });

    /// @test Matrix-matrix addition and subtraction.
    ///
    /// @f$\mathbf{A} + \mathbf{B} = \mathbf{A} + \mathbf{B} - \mathbf{B}@f$
    ret += test_property_n("{0} +- {0}"_format(mat_typestring),
                           mat_gen2,
                           lucid::plus,
                           [&](const Mat& testing, const auto& feed) noexcept {
                               const auto& [mat1, mat2] = feed;
                               const auto mm            = testing - mat2;
                               return assertion(mm, mat1);
                           });

    /// @test Inplace matrix-matrix addition and subtraction.
    ///
    /// @f$\mathbf{A} + \mathbf{B} = \mathbf{A} + \mathbf{B} - \mathbf{B}@f$
    ret += test_property_n(
        "{0} +-= {0}"_format(mat_typestring),
        mat_gen2,
        [](Mat mat1, const Mat& mat2) noexcept {
            mat1 += mat2;
            return mat1;
        },
        [&](Mat testing, const auto& feed) noexcept {
            const auto& [mat1, mat2] = feed;
            testing -= mat2;
            return assertion(testing, mat1);
        });

    // distribution to generate divizor that is guaranteed to be greater than zero
    std::uniform_real_distribution<T> divdist(T{1}, T{10000});

    auto sign_gen = [&, bdist = std::bernoulli_distribution(0.5)]() mutable noexcept {
        return static_cast<T>(minus_one_pow(bdist(g)));
    };

    auto div_vec_gen = [&]() noexcept { return Vec(generate<N>(divdist, g)) * sign_gen(); };

    std::uniform_int_distribution<size_t> rowdist(0, M - 1);

    /// @test Assignment to Matrix row from a vector.
    ///
    /// @f$\mathbf{M}_{m,n}[i] \gets \mathbf{V}_n(a_1,\cdots,a_n) =
    /// \begin{pmatrix}
    /// \cdots & \cdots & \cdots \\
    /// a_{i,1} & \cdots & a_{i,n} \\
    /// \cdots & \cdots & \cdots
    /// \end{pmatrix}@f$
    ret += test_property_n(
        "{}[] = Vector<{}, {}>"_format(mat_typestring, t_typestring, N),
        [&]() noexcept { return tuple(mat_gen(), vgen(), rowdist(g)); },
        [](Mat mat, const Vec& vec, const std::size_t idx) noexcept {
            mat[idx] = vec;
            return mat;
        },
        [](const Mat& testing, const auto& feed) noexcept {
            const auto& vec = get<1>(feed);
            const auto& idx = get<2>(feed);
            return any(testing[idx] != vec) || !all(lucid::isfinite(flat_ref(testing)));
        });

    /// @test Matrix row assignment-addition-subtraction
    ///
    /// @f$\mathbf{M}_{m,n}[i] \gets \mathbf{M}_{m,n}[i] \pm \mathbf{V}_n(a_1,\cdots,a_n)@f$
    ret += test_property_n(
        "{}[] +-= {}"_format(mat_typestring, vec_typestring),
        [&]() noexcept { return tuple(mat_gen(), vgen(), rowdist(g)); },
        [](Mat mat, const Vec& vec, const std::size_t idx) noexcept {
            mat[idx] += vec;
            return mat;
        },
        [&](Mat testing, const auto& feed) noexcept {
            const auto& [mat, vec, idx] = feed;
            testing[idx] -= vec;
            return assertion(testing, mat);
        });

    /// @test Matrix row assignment-multuply-subtraction
    ///
    /// @f$\mathbf{M}_{m,n}[i] \gets \mathbf{M}_{m,n}[i] \ast\div
    /// \mathbf{V}_n(a_1,\cdots,a_n)@f$
    ret += test_property_n(
        "{}[] */= {}"_format(mat_typestring, vec_typestring),
        [&]() noexcept { return tuple(mat_gen(), div_vec_gen(), rowdist(g)); },
        [](Mat mat, const Vec& vec, const std::size_t idx) noexcept {
            mat[idx] *= vec;
            return mat;
        },
        [&](Mat testing, const auto& feed) noexcept {
            const auto& [mat, vec, idx] = feed;
            testing[idx] /= vec;
            return assertion(testing, mat);
        });

    /// @test Matrix transposition and multiplication.
    ///
    /// @f$(\mathbf{AB})^T = \mathbf{B}^T\mathbf{A}@f$
    ret += test_property_n(
        "{}: transpose(A dot B) = transpose(B) dot transpose(A)"_format(mat_typestring),
        [&]() noexcept {
            return pair(Mat(generate<MN>(divdist, g)) * sign_gen(),
                        Matrix<T, N, M, array>(generate<MN>(divdist, g)) * sign_gen());
        },
        transpose ^ dot,
        [&](const auto& testing, const auto& feed) noexcept {
            const auto& ab_t   = testing;
            const auto& [a, b] = feed;
            const auto bt_at   = transpose(b) * transpose(a);
            return assertion(ab_t, bt_at);
        });

    if constexpr(M == 4)
    {
        const auto mat           = mat_gen();
        const auto& [a, b, c, d] = mat;
        const auto asrt = &std::get<0>(a) != &mat.at(0, 0) || &std::get<0>(b) != &mat.at(1, 0) ||
                          &std::get<0>(c) != &mat.at(2, 0) || &std::get<0>(d) != &mat.at(3, 0);

        ret += asrt;
    }

    return ret;
}

template <size_t M, size_t N, typename RandomEngine, typename... Ts>
unsigned
test_t(RandomEngine& g, const size_t num_tests, type_sequence<Ts...>) noexcept
{
    return (0u + ... + matrix_test<Ts, M, N>(g, num_tests));
}

template <size_t N, typename RandomEngine, size_t... Ms>
unsigned
test_m(RandomEngine& g, const size_t num_tests, index_sequence<Ms...>) noexcept
{
    return (0u + ... + test_t<Ms, N>(g, num_tests, ArithmeticTypes{}));
}

template <typename RandomEngine, size_t... Ns>
unsigned
test_n(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{
    return (0u + ... + test_m<Ns>(g, num_tests, Indicies{}));
}

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device rd;
    default_random_engine g(rd());

    const int ret = test_n(g, num_tests, Indicies{});

    if(ret)
        fmt::print("{} tests failed.\n", ret);
    else
        fmt::print("All tests passed.\n");

    return ret;
}
