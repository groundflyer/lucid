#include "property_test.hpp"

#include <base/utils.hpp>
#include <base/vector.hpp>
#include <base/rng.hpp>

#include <string>

using namespace std;
using namespace yapt;

static const constexpr size_t MaxN = 4;
using Indicies = make_index_sequence<MaxN - 1>;
using ArithmeticTypes = typelist<int, long, float, double>;

template <typename T, size_t N, typename RandomEngine>
auto
test_t_n(RandomEngine& g, const size_t num_tests) noexcept
{
    using Vec = Vector<T, N>;
    const auto t_typestring = get_typeinfo_string(T{});
    const auto vec_typestring = "Vector<{}, {}>"_format(t_typestring, N);
    RandomDistribution<T> dist(T{-10000}, T{10000});
    unsigned ret = 0;

    const auto test_property_n = [num_tests](auto&& ... args)
                                 { return test_property(num_tests, forward<decltype(args)>(args)...); };

    ret += test_property_n("{}({})"_format(vec_typestring, t_typestring),
                           [&](){ return dist(g); },
                           [](const auto& feed) { return Vec(feed); },
                           [](const auto& testing, const auto& feed) { return any(testing != feed); });

    // array generator
    auto argen = [&]() { return dist.template operator()<N>(g); };

    // array assertion
    const auto arass = [](const Vec& testing, const array<T, N>& feed) { return assert_tuples(testing, feed, make_index_sequence<N>{}); };

    ret += test_property_n("{}({})"_format(vec_typestring, get_typeinfo_string(array<T, N>{})),
                           argen,
                           [](const array<T, N>& feed){ return construct<Vec>(feed); },
                           arass);

    ret += test_property_n("{}(array<{}, {}>)"_format(vec_typestring, t_typestring, N),
                           argen,
                           [](const array<T, N>& feed) { return Vec(feed); },
                           arass);

    if constexpr (N > 2)
    {
        const constexpr auto N1 = N - 1;
        ret += test_property_n("{0}({1}, Vector<{1}, {2}>)"_format(vec_typestring, t_typestring, N1),
                               [&]() { return tuple(dist(g), Vector<T, N1>(dist.template operator()<N1>(g))); },
                               [](const auto& feed) { return construct<Vec>(feed); },
                               [](const Vec& testing, const auto& feed)
                               {
                                   const auto& [v0, vv] = feed;
                                   bool ret = testing[0] != v0;
                                   for(size_t i = 0; i < N1; ++i)
                                       ret |= vv[i] != testing[i + 1];
                                   return ret;
                               });
    }

    auto vgen = [&]() { return Vec(argen()); };

    // vector and scalar value generator
    auto vsgen = [&]() { return pair(vgen(), dist(g)); };

    const auto assertion = [](auto&& a, auto&& b)
                           {
                               if constexpr(is_floating_point_v<T>)
                                   return any(!almost_equal(forward<decltype(a)>(a), forward<decltype(b)>(b), 100 * N));
                               else
                                   return any(a != b);
                           };

    ret += test_property_n("{} +- {}"_format(vec_typestring, t_typestring),
                           vsgen,
                           [](const auto feed)
                           {
                               const auto& [vec, val] = feed;
                               return vec + val;
                           },
                           [&](const auto& testing, const auto feed)
                           {
                               const auto& [vec, val] = feed;
                               return assertion(testing - val, vec);
                           });

    ret += test_property_n("{} +-= {}"_format(vec_typestring, t_typestring),
                           vsgen,
                           [](const auto& feed)
                           {
                               auto [vec, val] = feed;
                               vec += val;
                               return vec;
                           },
                           [&](auto testing, const auto& feed)
                           {
                               auto [vec, val] = feed;
                               testing -= val;
                               return assertion(testing, vec);
                           });

    // distribution to generate divizor that is guaranteed to be greater than zero
    RandomDistribution<T> divdist(T{1}, T{10000});

    // random sign generator
    auto signgen = [&, bdist = RandomDistribution<bool>(0.5)]() mutable
                   { return static_cast<T>(math::minus_one_pow(bdist(g))); };

    // vector and divisor generator
    auto vdgen = [&]() { return pair(vgen(), divdist(g) * signgen()); };

    ret += test_property_n("{} */ {}"_format(vec_typestring, t_typestring),
                           vdgen,
                           [](const auto& feed)
                           {
                               const auto& [vec, val] = feed;
                               return vec * val;
                           },
                           [&](const auto& testing, const auto& feed)
                           {
                               const auto& [vec, val] = feed;
                               return assertion(testing / val, vec);
                           });

    ret += test_property_n("{} */= {}"_format(vec_typestring, t_typestring),
                           vdgen,
                           [](const auto& feed)
                           {
                               auto [vec, val] = feed;
                               vec *= val;
                               return vec;
                           },
                           [&](auto testing, const auto& feed)
                           {
                               const auto& [vec, val] = feed;
                               testing /= val;
                               return assertion(testing, vec);
                           });

    // pair of vectors generator
    auto vvgen = [&]() { return pair(vgen(), vgen()); };

    ret += test_property_n("{0} +- {0}"_format(vec_typestring),
                           vvgen,
                           [](const auto& feed)
                           {
                               const auto& [vec1, vec2] = feed;
                               return vec1 + vec2;
                           },
                           [&](const auto& testing, const auto& feed)
                           {
                               const auto& [vec1, vec2] = feed;
                               return assertion(testing - vec2, vec1);
                           });

    ret += test_property_n("{0} +-= {0}"_format(vec_typestring),
                           vvgen,
                           [](const auto& feed)
                           {
                               auto [vec1, vec2] = feed;
                               vec1 += vec2;
                               return vec1;
                           },
                           [&](auto testing, const auto& feed)
                           {
                               auto [vec1, vec2] = feed;
                               testing -= vec2;
                               return assertion(testing, vec1);
                           });

    // vector and vector-divizor generator
    auto vvdgen = [&]() { return pair(vgen(), Vec(divdist.template operator()<N>(g)) * signgen()); };

    ret += test_property_n("{0} */ {0}"_format(vec_typestring),
                           vvdgen,
                           [](const auto& feed)
                           {
                               const auto& [vec1, vec2] = feed;
                               return vec1 * vec2;
                           },
                           [&](const auto& testing, const auto& feed)
                           {
                               const auto& [vec1, vec2] = feed;
                               return assertion(testing / vec2, vec1);
                           });

    ret += test_property_n("{0} */= {0}"_format(vec_typestring),
                           vvdgen,
                           [](const auto& feed)
                           {
                               auto [vec1, vec2] = feed;
                               vec1 *= vec2;
                               return vec1;
                           },
                           [&](auto testing, const auto& feed)
                           {
                               const auto& [vec1, vec2] = feed;
                               testing /= vec2;
                               return assertion(testing, vec1);
                           });

    if constexpr (is_floating_point_v<T>)
    {
        ret += test_property_n("dot({0}, {0})|length({0})"_format(vec_typestring),
                               vgen,
                               [](const Vec& feed) { return dot(feed, feed); },
                               [](const T testing, const Vec& feed) { return !math::almost_equal(math::sqrt(testing), length(feed)); });

        ret += test_property_n("normalize({})"_format(vec_typestring),
                               vgen,
                               [](const Vec& feed) { return normalize(feed); },
                               [](const Vec& testing, Vec) { return !math::almost_equal(length(testing), T{1}); });

        if constexpr (N == 3)
            ret += test_property_n("cross({0}, {0})"_format(vec_typestring),
                                   [&]() { return pair(normalize(vgen()), normalize(vgen())); },
                                   [](const auto& feed)
                                   {
                                       const auto& [a, b] = feed;
                                       return cross(a, b);
                                   },
                                   [](const Vec& testing, const auto& feed)
                                   {
                                       const auto& [a, b] = feed;
                                       const auto& [ax, ay, az] = a;
                                       const auto& [bx, by, bz] = b;
                                       // 3-dimensional righthanded cross product
                                       const Vec check(ay * bz - az * by,
                                                       az * bx - ax * bz,
                                                       ax * by - ay * bx);
                                       return any(!almost_equal(testing, check));
                                   });
    }

    return ret;
}

template <size_t N, typename RandomEngine, typename... Ts>
auto
test_t(RandomEngine& g, const size_t num_tests, typelist<Ts...>) noexcept
{ return (0u + ... + test_t_n<Ts, N>(g, num_tests)); }

template <typename RandomEngine, size_t... Ns>
auto
test_n(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + test_t<Ns + 2>(g, num_tests, ArithmeticTypes{})); }


template <size_t N, typename RandomEngine>
auto
__boolean_test(RandomEngine& g, const size_t num_tests) noexcept
{
    RandomDistribution<bool> dist(0.5);
    unsigned ret = 0;

    auto test_property_n = [num_tests](auto&& ... args)
                           { return test_property(num_tests, forward<decltype(args)>(args)...); };

    ret += test_property_n("any(Vector<bool, {}>)"_format(N),
                           [&]() { return Vector(dist.template operator()<N>(g)); },
                           [](const Vector<bool, N>& feed) { return any(feed); },
                           [](const bool testing, const Vector<bool, N>& feed)
                           {
                               return testing != apply([](auto... vals) { return (false || ... || vals); },
                                                        feed.data());
                           });

    ret += test_property_n("all(Vector<bool, {}>)"_format(N),
                           [&]() { return Vector(dist.template operator()<N>(g)); },
                           [](const Vector<bool, N>& feed) { return all(feed); },
                           [](const bool testing, const Vector<bool, N>& feed)
                           {
                               return testing != apply([](auto... vals) { return (true && ... && vals); },
                                                        feed.data());
                           });

    return ret;
}

template <typename RandomEngine, size_t... Ns>
auto
boolean_test(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + __boolean_test<Ns + 2>(g, num_tests)); }

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    int ret = 0;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    Indicies idxs;

    ret += test_n(g, num_tests, idxs);
    ret += boolean_test(g, num_tests, idxs);

    auto log = spdlog::get("debug");
    if(ret)
        log->info("{} tests failed.", ret);
    else
        log->info("All tests passed successfully.");

    return ret;
}
