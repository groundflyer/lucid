#include "property_test.hpp"

#include <base/matrix.hpp>
#include <base/utils.hpp>
#include <base/rng.hpp>

using namespace std;
using namespace yapt;

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
    unsigned ret = 0;

    auto test_property_n = [num_tests](auto&& ... args)
                           { return test_property(num_tests, forward<decltype(args)>(args)...); };

    ret += test_property_n("{}({}).flat_ref()"_format(mat_typestring, t_typestring),
                           [&](){ return dist(g); },
                           [](const auto feed){ return Mat(feed); },
                           [](auto&& property, auto&& feed){ return any(property.flat_ref() != feed); });

    const constexpr auto MN = M * N;
    auto argen = [&]() { return dist.template operator()<MN>(g); };
    auto arass = [](const Mat property, const auto feed)
                 {
                     bool ret = false;
                     for(size_t i = 0; i < MN; ++i)
                         ret |= property.at(i) != feed[i];
                     return ret;
                 };

    ret += test_property_n("{}({})"_format(mat_typestring, get_typeinfo_string(array<T, MN>{})),
                           argen,
                           [](const auto& feed) { return construct<Mat>(feed); },
                           arass);

    ret += test_property_n("{}(array<{}, {}>)"_format(mat_typestring, t_typestring, N),
                           argen,
                           [](const auto& feed) { return Mat(feed); },
                           arass);

    ret += test_property_n("{}({})"_format(mat_typestring, get_typeinfo_string(array<Vec, M>{})),
                           [&]()
                           {
                               array<Vec, M> ret;
                               for(size_t i = 0; i < M; ++i)
                                   ret[i] = Vec(dist.template operator()<N>(g));
                               return ret;
                           },
                           [](const auto& feed) { return construct<Mat>(feed); },
                           [](const Mat& property, const auto& feed)
                           {
                               bool ret = false;
                               for(size_t i = 0; i < M; ++i)
                                   ret |= any(property[i] != feed[i]);
                               return ret;
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
                                   ret[i] = dist.template operator()<N>(g);
                               return ret;
                           },
                           [](const auto& feed) { return construct<Mat>(feed); },
                           [](const Mat& property, const auto& feed)
                           {
                               bool ret = false;
                               for(size_t i = 0; i < M; ++i)
                                   for(size_t j = 0; j < N; ++j)
                                       ret |= property[i][j] != feed[i][j];
                               return ret;
                           });

    if constexpr (M > 2 && N > 2)
    {
        const constexpr auto M1 = M - 1;
        const constexpr auto N1 = N - 1;
        const constexpr auto MN1 = M1 * N1;
        using sMat = Matrix<T, M1, N1>;
        using sVec = Vector<T, N1>;
        auto smatgen = [&](){ return sMat(dist.template operator()<MN1>(g)); };
        auto svecgen = [&](){ return sVec(dist.template operator()<N1>(g)); };

        ret += test_property_n("{}(Matrix<{}, {}, {}>)"_format(mat_typestring, t_typestring, M1, N1),
                               smatgen,
                               [](const sMat feed) { return Mat(feed); },
                               [](const Mat property, const sMat feed)
                               {
                                   bool ret = false;
                                   for(size_t i = 0; i < M1; ++i)
                                       for(size_t j = 0; j < N1; ++j)
                                           ret |= feed.at(i, j) != property.at(i, j);
                                   return ret;
                               });

        ret += test_property_n("{0}({1}, Vector<{1}, {2}>, Matrix<{1}, {3}, {2}>)"_format(mat_typestring, t_typestring, N1, M1),
                               [&](){ return tuple(dist(g), svecgen(), smatgen()); },
                               [](const auto feed) { return construct<Mat>(feed); },
                               [](const Mat property, const auto feed)
                               {
                                   const auto& [scalar, vec, mat] = feed;
                                   bool ret = property.at(0) != scalar;
                                   for(size_t i = 0; i < N1; ++i)
                                       ret |= vec[i] != property[0][i + 1];
                                   for(size_t i = 0; i < M1; ++i)
                                       for(size_t j = 0; j < N1; ++j)
                                           ret |= mat.at(i, j) != property.at(i+1, j);
                                   return ret;
                               });
    }

    return ret;
}

template <size_t M, size_t N, typename RandomEngine, typename... Ts>
auto
test_t(RandomEngine& g, const size_t num_tests, typelist<Ts...>) noexcept
{ return (0u + ... + test_t_r_c<Ts, M, N>(g, num_tests)); }

template <size_t N, typename RandomEngine, size_t... Ms>
auto
test_m(RandomEngine& g, const size_t num_tests, index_sequence<Ms...>) noexcept
{ return (0u + ... + test_t<Ms + 2, N>(g, num_tests, ArithmeticTypes{})); }

template <typename RandomEngine, size_t... Ns>
auto
test_n(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + test_m<Ns + 2>(g, num_tests, Indicies{})); }

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    int ret = 0;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    ret += test_n(g, num_tests, Indicies{});

    // std::cout << std::boolalpha;
    // TEST_AND_COUT((is_array_v<Mat4f>));
    // TEST_AND_COUT((is_scalar_v<Mat4f>));
    // TEST_AND_COUT(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));
    // auto m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    // TEST_AND_COUT(m[0]);
    // TEST_AND_COUT(sizeof(Matrix<unsigned char,4,4,array>(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)));
    // TEST_AND_COUT(Mat4f::unit());
    // auto u = m;

	// const auto cu = m;
	// TEST_AND_COUT(cu[0]);
	// TEST_AND_COUT((is_same_v<decltype(cu[0]), const Vector<float, 4, ArrayView>>));

	// auto& [cu0, cu1, cu2, cu3] = cu;
	// TEST_AND_COUT((is_same_v<decltype(cu0), const Vector<float, 4, ArrayView>>));
	// TEST_AND_COUT((cu0.cbegin() == cu.cbegin()));
	
	// auto u00 = u[0];
	// TEST_AND_COUT((is_same_v<decltype(u00), Vector<float, 4, ArrayView>>));
	// auto& [u0, u1, u2, u3] = u;
	// TEST_AND_COUT((u00.cbegin() == u0.cbegin()));
	// TEST_AND_COUT((is_same_v<decltype(u00), decltype(u0)>));

	// auto [v0, v1, v2, v3] = u;
	// TEST_AND_COUT((u00.cbegin() == v0.cbegin()));
	// TEST_AND_COUT((is_same_v<decltype(u00), decltype(v0)>));

    // u[0][0] = 10;
	// u0 += 100;
	// u00 += 1000;
    // u[0] += 2;
    // u[1] = Vec4f(58,59,60,61);
    // u[2][3] = 34;
    // u[3] *= Vec4f(15, -5, 0.5, 31);
    // TEST_AND_COUT(u);
    // COUT_ARRAY(u.data());
	// TEST_AND_COUT(Mat4f(cu0, cu1, cu2, cu3));
    // TEST_AND_COUT(Mat4f(Vec4f(1,0,-2,4), Vec4f(0,1,0,3), Vec4f(0,0,1,0), Vec4f(0,-5,0,1)));
    // TEST_AND_COUT(Mat3f(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)));
    // TEST_AND_COUT(Mat4f(Mat3f(1,2,3,4,5,6,7,8,9)));
    // float a[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    // COUT_ARRAY(a);
    // TEST_AND_COUT(Mat4fe(ExArr16f(a)));
    // auto extMat = Mat4fe(ExArr16f(a));
    // extMat = Mat4f(12);
    // TEST_AND_COUT(extMat);
    // u = extMat;
    // TEST_AND_COUT(u);
    // extMat = 1;
    // extMat += Mat4f::unit();
    // extMat *= 10;
    // extMat /= 2;
    // TEST_AND_COUT(extMat);
    // u = Mat4f(10);
    // TEST_AND_COUT(u);
    // TEST_AND_COUT((Mat4f(10) + extMat));
    // TEST_AND_COUT((Mat4f(10) + 1));
    // TEST_AND_COUT((Mat4f(1) * -3.4));
    // TEST_AND_COUT((Mat4f(10).dot(Vec4f(5))));
    // TEST_AND_COUT((Mat4f::unit().dot(Mat4f(5))));
    // TEST_AND_COUT((Mat4f(10) / 2));
    // TEST_AND_COUT(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16).transpose());
    // TEST_AND_COUT(minor_matrix((Mat4f::unit() + 1) * 2, 3, 3));
    // TEST_AND_COUT(cofactor(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)));
    // TEST_AND_COUT(det(Mat4f::unit()));
    // TEST_AND_COUT(det((Mat4f::unit() + 1) * 2));

    // bool invertible = det((Mat4f::unit() + 1) * 2) != 0;
    // cout << "Is m2 invertible? "
    // 	 << (invertible ? "Yes" : "No") << endl;
    // if (invertible)
    // {
    // 	TEST_AND_COUT(inverse((Mat4f::unit() + 1) * 2));
    // 	TEST_AND_COUT(transpose((Mat4f::unit() + 1) * 2));
    // 	TEST_AND_COUT((((Mat4f::unit() + 1) * 2).dot(inverse((Mat4f::unit() + 1) * 2))));
    // 	TEST_AND_COUT(det((((Mat4f::unit() + 1) * 2).dot(inverse((Mat4f::unit() + 1) * 2)))));
    // }

    // TEST_AND_COUT((Matrix<float, 16, 32, array>(0.16f).dot(Matrix<float, 32, 12, array>(-4.5f))));
    // TEST_AND_COUT((Matrix<float, 2, 4, array>::unit() > Matrix<float, 2, 4, array>(0.5)));

    return ret;
}
