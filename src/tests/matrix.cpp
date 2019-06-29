#include "property_test.hpp"

#include <utils/typelist.hpp>
#include <base/matrix.hpp>
#include <base/rng.hpp>

using namespace std;
using namespace lucid;

static const constexpr size_t MaxN = 4;
using Indicies = make_index_sequence<MaxN - 1>;
using ArithmeticTypes = typelist<int, long, float, double>;

template <typename T, size_t M, size_t N, typename RandomEngine>
auto
test_t_r_c(RandomEngine& g, const size_t num_tests) noexcept
{
    using Mat = Matrix<T, M, N>;
    using Vec = Vector<T, N>;
    const auto t_typestring = get_typeinfo_string(T{});
    const auto mat_typestring = "Matrix<{}, {}, {}>"_format(t_typestring, M, N);
    RandomDistribution<T> dist(T{-10000}, T{10000});
    static const constexpr auto MN = M * N;
    auto array_mn_gen = [&]() { return dist.template operator()<MN>(g); };
    const auto array_assertion = [](const Mat& testing, const auto& feed)
                                 {
                                     bool ret = false;
                                     for(size_t i = 0; i < MN; ++i)
                                         ret |= testing.at(i) != feed[i];
                                     return ret || !all(lucid::isfinite(flat_ref(testing)));
                                 };
    auto array_n_gen = [&](){ return dist.template operator()<N>(g); };
    auto vgen = [&](){ return Vec(array_n_gen()); };

    const constexpr double threshold = is_floating_point_v<T> ? 0.01 : 0.0;
    const auto test_property_n = [num_tests, threshold](auto&& ... args)
                                 { return test_property(num_tests, threshold, forward<decltype(args)>(args)...); };

    const auto assertion = [](const auto& a, const auto& b)
                           {
                               if constexpr(is_floating_point_v<T>)
                               {
                                   const constexpr unsigned ULP = 5;
                                   return any(!almost_equal(flat_ref(a), flat_ref(b), ULP))
                                       || !all(lucid::isfinite(flat_ref(a)))
                                       || !all(lucid::isfinite(flat_ref(b)));
                               }
                               else
                                   return any(flat_ref(a) != flat_ref(b));
                           };

    auto mat_gen = [&]() { return Mat(array_mn_gen()); };

    unsigned ret = 0;

    ret += test_property_n("flat_ref({}({}))"_format(mat_typestring, t_typestring),
                           [&](){ return dist(g); },
                           [](const auto feed){ return Mat(feed); },
                           [](const auto& testing, const auto& feed)
                           { return any(flat_ref(testing) != feed) || !all(lucid::isfinite(flat_ref(testing))); });

    ret += test_property_n("{}({})"_format(mat_typestring, get_typeinfo_string(array<T, MN>{})),
                           array_mn_gen,
                           [](const auto& feed) { return std::make_from_tuple<Mat>(feed); },
                           array_assertion);

    ret += test_property_n("{}(array<{}, {}>)"_format(mat_typestring, t_typestring, N),
                           array_mn_gen,
                           [](const auto& feed) { return Mat(feed); },
                           array_assertion);

    ret += test_property_n("{}({})"_format(mat_typestring, get_typeinfo_string(array<Vec, M>{})),
                           [&]()
                           {
                               array<Vec, M> ret;
                               for(size_t i = 0; i < M; ++i)
                                   ret[i] = vgen();
                               return ret;
                           },
                           [](const auto& feed) { return std::make_from_tuple<Mat>(feed); },
                           [](const Mat& testing, const auto& feed)
                           {
                               bool ret = false;
                               for(size_t i = 0; i < M; ++i)
                                   ret |= any(testing[i] != feed[i]);
                               return ret || !all(lucid::isfinite(flat_ref(testing)));
                           });

    stringstream ss;
    const auto at_string = "array<{}, {}>"_format(t_typestring, N);
    for(size_t i = 0; i < M - 1; ++i)
        ss << at_string << ", ";
    ss << at_string;
    ret += test_property_n("{}({})"_format(mat_typestring, ss.str()),
                           [&]()
                           {
                               array<array<T, N>, M> ret;
                               for(size_t i = 0; i < M; ++i)
                                   ret[i] = array_n_gen();
                               return ret;
                           },
                           [](const auto& feed) { return std::make_from_tuple<Mat>(feed); },
                           [](const Mat& testing, const auto& feed)
                           {
                               bool ret = false;
                               for(size_t i = 0; i < M; ++i)
                                   for(size_t j = 0; j < N; ++j)
                                       ret |= testing[i][j] != feed[i][j];
                               return ret || !all(lucid::isfinite(flat_ref(testing)));
                           });

    if constexpr (M > 2 && N > 2)
    {
        const constexpr auto M1 = M - 1;
        const constexpr auto N1 = N - 1;
        const constexpr auto MN1 = M1 * N1;
        using sMat = Matrix<T, M1, N1>;
        using sVec = Vector<T, N1>;
        auto smat_gen = [&](){ return sMat(dist.template operator()<MN1>(g)); };
        auto svecgen = [&](){ return sVec(dist.template operator()<N1>(g)); };

        ret += test_property_n("{}(Matrix<{}, {}, {}>)"_format(mat_typestring, t_typestring, M1, N1),
                               smat_gen,
                               [](const sMat& feed) { return Mat(feed); },
                               [](const Mat& testing, const sMat& feed)
                               {
                                   bool ret = false;
                                   for(size_t i = 0; i < M1; ++i)
                                       for(size_t j = 0; j < N1; ++j)
                                           ret |= feed.at(i, j) != testing.at(i, j);
                                   return ret || !all(lucid::isfinite(flat_ref(testing)));
                               });

        ret += test_property_n("{0}({1}, Vector<{1}, {2}>, Matrix<{1}, {3}, {2}>)"_format(mat_typestring, t_typestring, N1, M1),
                               [&](){ return tuple(dist(g), svecgen(), smat_gen()); },
                               [](const auto& feed) { return std::make_from_tuple<Mat>(feed); },
                               [](const Mat& testing, const auto& feed)
                               {
                                   const auto& [scalar, vec, mat] = feed;
                                   bool ret = testing.at(0) != scalar;
                                   for(size_t i = 0; i < N1; ++i)
                                       ret |= vec[i] != testing[0][i + 1];
                                   for(size_t i = 0; i < M1; ++i)
                                       for(size_t j = 0; j < N1; ++j)
                                           ret |= mat.at(i, j) != testing.at(i+1, j);
                                   return ret;
                               });

        if constexpr (M == N && is_floating_point_v<T>)
            ret += test_property_n("{0}: inverse(A) dot A = {0}::identity()"_format(mat_typestring),
                                   [&](){ return mat_gen(); },
                                   [](const auto& feed){ return inverse(feed); },
                                   [](const auto& testing, const auto& feed)
                                   {
                                       // identity mantrix elements have different magnitude
                                       // equalizing them as ulp is same for all of them
                                       const auto zero = Mat::identity() - feed.dot(testing);
                                       const constexpr auto ulp = pow<sizeof(T)>(100ul);
                                       return any(!almost_equal(flat_ref(zero), T{0}, ulp)) || !all(lucid::isfinite(flat_ref(testing)));
                                   });
    }

    ret += test_property_n("{} +- {}"_format(mat_typestring, t_typestring),
                           [&](){ return pair(mat_gen(), dist(g)); },
                           [](const auto& feed)
                           {
                               const auto& [mat, scalar] = feed;
                               return mat + scalar;
                           },
                           [&](const Mat& testing, const auto& feed)
                           {
                               const auto& [mat, scalar] = feed;
                               const auto mm = testing - scalar;
                               return assertion(mm, mat);
                           });

    ret += test_property_n("{} +-= {}"_format(mat_typestring, t_typestring),
                           [&](){ return pair(mat_gen(), dist(g)); },
                           [](const auto& feed)
                           {
                               auto [mat, scalar] = feed;
                               mat += scalar;
                               return mat;
                           },
                           [&](Mat testing, const auto& feed)
                           {
                               const auto& [mat, scalar] = feed;
                               testing -= scalar;
                               return assertion(testing, mat);
                           });

    ret += test_property_n("{0} +- {0}"_format(mat_typestring),
                           [&](){ return pair(mat_gen(), mat_gen()); },
                           [](const auto& feed)
                           {
                               const auto& [mat1, mat2] = feed;
                               return mat1 + mat2;
                           },
                           [&](const Mat& testing, const auto& feed)
                           {
                               const auto& [mat1, mat2] = feed;
                               const auto mm = testing - mat2;
                               return assertion(mm, mat1);
                           });

    ret += test_property_n("{0} +-= {0}"_format(mat_typestring),
                           [&](){ return pair(mat_gen(), mat_gen()); },
                           [](const auto& feed)
                           {
                               auto [mat1, mat2] = feed;
                               mat1 += mat2;
                               return mat1;
                           },
                           [&](Mat testing, const auto& feed)
                           {
                               const auto& [mat1, mat2] = feed;
                               testing -= mat2;
                               return assertion(testing, mat1);
                           });

    // distribution to generate divizor that is guaranteed to be greater than zero
    RandomDistribution<T> divdist(T{1}, T{10000});

    auto sign_gen = [&, bdist = RandomDistribution<bool>(0.5)]() mutable
                   { return static_cast<T>(minus_one_pow(bdist(g))); };

    auto div_scalar_gen = [&](){ return divdist(g) * sign_gen(); };
    auto div_mat_gen = [&](){ return Mat(divdist.template operator()<MN>(g)) * sign_gen(); };
    auto div_vec_gen = [&](){ return Vec(divdist.template operator()<N>(g)) * sign_gen(); };

    ret += test_property_n("{} */ {}"_format(mat_typestring, t_typestring),
                           [&](){ return pair(mat_gen(),div_scalar_gen()); },
                           [](const auto& feed)
                           {
                               const auto& [mat, scalar] = feed;
                               return mat * scalar;
                           },
                           [&](const Mat& testing, const auto& feed)
                           {
                               const auto& [mat, scalar] = feed;
                               const auto mm = testing / scalar;
                               return assertion(mm, mat);
                           });

    ret += test_property_n("{} */= {}"_format(mat_typestring, t_typestring),
                           [&](){ return pair(mat_gen(),div_scalar_gen()); },
                           [](const auto& feed)
                           {
                               auto [mat, scalar] = feed;
                               mat *= scalar;
                               return mat;
                           },
                           [&](Mat testing, const auto& feed)
                           {
                               const auto& [mat, scalar] = feed;
                               testing /= scalar;
                               return assertion(testing, mat);
                           });

    ret += test_property_n("{0} */ {0}"_format(mat_typestring),
                           [&](){ return pair(mat_gen(),div_mat_gen()); },
                           [](const auto& feed)
                           {
                               const auto& [mat1, mat2] = feed;
                               return mat1 * mat2;
                           },
                           [&](const Mat& testing, const auto& feed)
                           {
                               const auto& [mat1, mat2] = feed;
                               const auto mm = testing / mat2;
                               return assertion(mm, mat1);
                           });

    ret += test_property_n("{0} */= {0}"_format(mat_typestring),
                           [&](){ return pair(mat_gen(),div_mat_gen()); },
                           [](const auto& feed)
                           {
                               auto [mat1, mat2] = feed;
                               mat1 *= mat2;
                               return mat1;
                           },
                           [&](Mat testing, const auto& feed)
                           {
                               const auto& [mat1, mat2] = feed;
                               testing /= mat2;
                               return assertion(testing, mat1);
                           });

    RandomDistribution<size_t> rowdist(0, M-1);

    ret += test_property_n("{}[] = Vector<{}, {}>"_format(mat_typestring, t_typestring, N),
                           [&](){ return tuple(mat_gen(), vgen(), rowdist(g)); },
                           [](auto feed)
                           {
                               auto& [mat, vec, idx] = feed;
                               mat[idx] = vec;
                               return mat;
                           },
                           [](const Mat& testing, const auto& feed)
                           {
                               const auto& vec = get<1>(feed);
                               const auto& idx = get<2>(feed);
                               return any(testing[idx] != vec) || !all(lucid::isfinite(flat_ref(testing)));
                           });

    ret += test_property_n("{}[] +-= Vector<{}, {}>"_format(mat_typestring, t_typestring, N),
                           [&](){ return tuple(mat_gen(), vgen(), rowdist(g)); },
                           [](auto feed)
                           {
                               auto& [mat, vec, idx] = feed;
                               mat[idx] += vec;
                               return mat;
                           },
                           [&](Mat testing, const auto& feed)
                           {
                               const auto& [mat, vec, idx] = feed;
                               testing[idx] -= vec;
                               return assertion(testing, mat);
                           });

    ret += test_property_n("{}[] */= Vector<{}, {}>"_format(mat_typestring, t_typestring, N),
                           [&](){ return tuple(mat_gen(),div_vec_gen(), rowdist(g)); },
                           [](auto feed)
                           {
                               auto& [mat, vec, idx] = feed;
                               mat[idx] *= vec;
                               return mat;
                           },
                           [&](Mat testing, const auto& feed)
                           {
                               const auto& [mat, vec, idx] = feed;
                               testing[idx] /= vec;
                               return assertion(testing, mat);
                           });

    ret += test_property_n("{}: transpose(A dot B) = transpose(B) dot transpose(A)"_format(mat_typestring),
                           [&](){ return pair(div_mat_gen(), Matrix<T, N, M>(divdist.template operator()<MN>(g)) * sign_gen()); },
                           [](const auto& feed)
                           {
                               const auto& [a, b] = feed;
                               return transpose(a.dot(b));
                           },
                           [&](const auto& testing, const auto& feed)
                           {
                               const auto& ab_t = testing;
                               const auto& [a, b] = feed;
                               const auto bt_at = transpose(b).dot(transpose(a));
                               return assertion(ab_t, bt_at);
                           });

    if constexpr (M == 4)
    {
        const auto mat = mat_gen();
        const auto& [a, b, c, d] = mat;
        const auto asrt =
            &std::get<0>(a) != &mat.at(0, 0) ||
            &std::get<0>(b) != &mat.at(1, 0) ||
            &std::get<0>(c) != &mat.at(2, 0) ||
            &std::get<0>(d) != &mat.at(3, 0);

        ret += asrt;
    }

    return ret;
}

template <size_t M, size_t N, typename RandomEngine, typename... Ts>
auto
test_t(RandomEngine& g, const size_t num_tests, typelist<Ts...>) noexcept
{ return (0 + ... + test_t_r_c<Ts, M, N>(g, num_tests)); }

template <size_t N, typename RandomEngine, size_t... Ms>
auto
test_m(RandomEngine& g, const size_t num_tests, index_sequence<Ms...>) noexcept
{ return (0 + ... + test_t<Ms + 2, N>(g, num_tests, ArithmeticTypes{})); }

template <typename RandomEngine, size_t... Ns>
auto
test_n(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0 + ... + test_m<Ns + 2>(g, num_tests, Indicies{})); }

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
