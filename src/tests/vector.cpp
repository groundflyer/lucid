#include "property_test.hpp"

#include <base/rng.hpp>
#include <base/vector.hpp>
#include <utils/typelist.hpp>

#include <cstdlib>

using namespace std;
using namespace lucid;

static const constexpr size_t MaxN = 4;
using Indicies                     = make_index_sequence<MaxN - 1>;
using ArithmeticTypes              = typelist<int, long, float, double>;

template <typename T, size_t N, typename RandomEngine>
auto
test_t_n(RandomEngine& g, const size_t num_tests) noexcept
{
    using Vec                            = Vector<T, N>;
    const auto            t_typestring   = get_typeinfo_string(T{});
    const auto            vec_typestring = get_typeinfo_string(Vec{});
    const auto            arr_typestring = get_typeinfo_string(Vec().data());
    RandomDistribution<T> dist(T{-10000}, T{10000});
    unsigned              ret = 0;

    const constexpr double threshold       = is_floating_point_v<T> ? 0.01 : 0.0;
    const auto             test_property_n = [num_tests, threshold](auto&&... args) {
        return test_property(num_tests, threshold, forward<decltype(args)>(args)...);
    };

    ret += test_property_n(
        "{}({})"_format(vec_typestring, t_typestring),
        [&]() { return dist(g); },
        [](const auto& feed) { return Vec(feed); },
        [](const auto& testing, const auto& feed) { return any(testing != feed); });

    // array generator
    auto argen = [&]() { return dist.template operator()<N>(g); };

    // array assertion
    const auto arass = [](const Vec& testing, const array<T, N>& feed) {
        return assert_tuples(testing, feed, make_index_sequence<N>{}) &&
               !all(lucid::isfinite(testing));
    };

    ret += test_property_n(
        "{}({})"_format(vec_typestring, arr_typestring),
        argen,
        [](const array<T, N>& feed) { return std::make_from_tuple<Vec>(feed); },
        arass);

    ret += test_property_n(
        "{}({})"_format(vec_typestring, arr_typestring, N),
        argen,
        [](const array<T, N>& feed) { return Vec(feed); },
        arass);

    if constexpr(N > 2)
    {
        const constexpr auto N1 = N - 1;
        ret += test_property_n(
            "{0}({1}, Vector<{1}, {2}>)"_format(vec_typestring, t_typestring, N1),
            [&]() { return tuple(dist(g), Vector<T, N1>(dist.template operator()<N1>(g))); },
            [](const auto& feed) { return std::make_from_tuple<Vec>(feed); },
            [](const Vec& testing, const auto& feed) {
                const auto& [v0, vv] = feed;
                bool ret             = testing[0] != v0;
                for(size_t i = 0; i < N1; ++i) ret |= vv[i] != testing[i + 1];
                return ret || !all(lucid::isfinite(testing));
            });
    }

    auto vgen = [&]() { return Vec(argen()); };

    // vector and scalar value generator
    auto vsgen = [&]() { return pair(vgen(), dist(g)); };

    const auto assertion = [](auto&& a, auto&& b) {
        if constexpr(is_floating_point_v<T>)
        {
            static const constexpr unsigned ULP = 200;
            return any(!almost_equal(forward<decltype(a)>(a), forward<decltype(b)>(b), ULP)) ||
                   !all(lucid::isfinite(a)) || !all(lucid::isfinite(b));
        }
        else
            return any(a != b);
    };

    ret += test_property_n(
        "{} +- {}"_format(vec_typestring, t_typestring),
        vsgen,
        [](const auto feed) {
            const auto& [vec, val] = feed;
            return vec + val;
        },
        [&](const auto& testing, const auto feed) {
            const auto& [vec, val] = feed;
            return assertion(testing - val, vec);
        });

    ret += test_property_n(
        "{} +-= {}"_format(vec_typestring, t_typestring),
        vsgen,
        [](const auto& feed) {
            auto [vec, val] = feed;
            vec += val;
            return vec;
        },
        [&](auto testing, const auto& feed) {
            auto [vec, val] = feed;
            testing -= val;
            return assertion(testing, vec);
        });

    // distribution to generate divizor that is guaranteed to be greater than zero
    RandomDistribution<T> divdist(T{1}, T{10000});

    // random sign generator
    auto signgen = [&, bdist = RandomDistribution<bool>(0.5)]() mutable {
        return static_cast<T>(minus_one_pow(bdist(g)));
    };

    // vector and divisor generator
    auto vdgen = [&]() { return pair(vgen(), divdist(g) * signgen()); };

    ret += test_property_n(
        "{} */ {}"_format(vec_typestring, t_typestring),
        vdgen,
        [](const auto& feed) {
            const auto& [vec, val] = feed;
            return vec * val;
        },
        [&](const auto& testing, const auto& feed) {
            const auto& [vec, val] = feed;
            return assertion(testing / val, vec);
        });

    ret += test_property_n(
        "{} */= {}"_format(vec_typestring, t_typestring),
        vdgen,
        [](const auto& feed) {
            auto [vec, val] = feed;
            vec *= val;
            return vec;
        },
        [&](auto testing, const auto& feed) {
            const auto& [vec, val] = feed;
            testing /= val;
            return assertion(testing, vec);
        });

    // pair of vectors generator
    auto vvgen = [&]() { return pair(vgen(), vgen()); };

    ret += test_property_n(
        "{0} +- {0}"_format(vec_typestring),
        vvgen,
        [](const auto& feed) {
            const auto& [vec1, vec2] = feed;
            return vec1 + vec2;
        },
        [&](const auto& testing, const auto& feed) {
            const auto& [vec1, vec2] = feed;
            return assertion(testing - vec2, vec1);
        });

    ret += test_property_n(
        "{0} +-= {0}"_format(vec_typestring),
        vvgen,
        [](const auto& feed) {
            auto [vec1, vec2] = feed;
            vec1 += vec2;
            return vec1;
        },
        [&](auto testing, const auto& feed) {
            auto [vec1, vec2] = feed;
            testing -= vec2;
            return assertion(testing, vec1);
        });

    ret += test_property_n(
        "min({0}, {0}), max({0}, {0})"_format(vec_typestring),
        vvgen,
        [](const auto& feed) {
            const auto& [vec1, vec2] = feed;
            return pair{lucid::min(vec1, vec2), lucid::max(vec1, vec2)};
        },
        [&](const auto& testing, const auto&) {
            const auto& [vmin, vmax] = testing;
            return any(vmin > vmax) || !all(lucid::isfinite(vmin)) || !all(lucid::isfinite(vmax));
        });

    // vector and vector-divizor generator
    auto vvdgen = [&]() {
        return pair(vgen(), Vec(divdist.template operator()<N>(g)) * signgen());
    };

    ret += test_property_n(
        "{0} */ {0}"_format(vec_typestring),
        vvdgen,
        [](const auto& feed) {
            const auto& [vec1, vec2] = feed;
            return vec1 * vec2;
        },
        [&](const auto& testing, const auto& feed) {
            const auto& [vec1, vec2] = feed;
            return assertion(testing / vec2, vec1);
        });

    ret += test_property_n(
        "{0} */= {0}"_format(vec_typestring),
        vvdgen,
        [](const auto& feed) {
            auto [vec1, vec2] = feed;
            vec1 *= vec2;
            return vec1;
        },
        [&](auto testing, const auto& feed) {
            const auto& [vec1, vec2] = feed;
            testing /= vec2;
            return assertion(testing, vec1);
        });

    if constexpr(is_floating_point_v<T>)
    {
        ret += test_property_n(
            "{}: A dot A = length A"_format(vec_typestring),
            vgen,
            [](const Vec& feed) { return dot(feed, feed); },
            [](const T testing, const Vec& feed) {
                return !almost_equal(math::sqrt(testing), length(feed), 5) ||
                       !std::isfinite(testing);
            });

        ret += test_property_n(
            "normalize({}) = 1"_format(vec_typestring),
            vgen,
            [](const Vec& feed) { return normalize(feed); },
            [](const Vec& testing, Vec) {
                return !almost_equal(length(testing), T{1}, 5) || !all(lucid::isfinite(testing));
            });

        if constexpr(N > 2)
            ret += test_property(
                num_tests,
                0.01,
                "{}: C <- cross A B | A dot C = B dot C = 0"_format(vec_typestring),
                [&]() { return pair(normalize(vgen()), normalize(vgen())); },
                [](const auto& feed) {
                    const auto& [a, b] = feed;
                    return cross(a, b);
                },
                [](const Vec& testing, const auto& feed) {
                    const auto& [a, b]       = feed;
                    const auto           at  = a.dot(testing);
                    const auto           bt  = b.dot(testing);
                    const constexpr auto ulp = pow<sizeof(T)>(is_same_v<T, double> ? 100ul : 55u);
                    return !(almost_equal(at, T{0}, ulp) || almost_equal(bt, T{0}, ulp)) ||
                           !all(lucid::isfinite(testing));
                });
    }

    // structure binding test
    if constexpr(N == 4)
    {
        const auto vec           = vgen();
        const auto& [x, y, z, w] = vec;
        const auto asrt          = &x != &std::get<0>(vec) || &y != &std::get<1>(vec) ||
                          &z != &std::get<2>(vec) || &w != &std::get<3>(vec);

        ret += asrt;
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
    unsigned                 ret            = 0;
    auto                     vec_typestring = get_typeinfo_string(Vector<bool, N>{});

    static const constexpr double threshold       = 0.0;
    auto                          test_property_n = [num_tests](auto&&... args) {
        return test_property(num_tests, threshold, forward<decltype(args)>(args)...);
    };

    ret += test_property_n(
        "any({})"_format(vec_typestring),
        [&]() { return Vector(dist.template operator()<N>(g)); },
        [](const Vector<bool, N>& feed) { return any(feed); },
        [](const bool testing, const Vector<bool, N>& feed) {
            return testing !=
                   apply([](auto... vals) { return (false || ... || vals); }, feed.data());
        });

    ret += test_property_n(
        "all({})"_format(vec_typestring),
        [&]() { return Vector(dist.template operator()<N>(g)); },
        [](const Vector<bool, N>& feed) { return all(feed); },
        [](const bool testing, const Vector<bool, N>& feed) {
            return testing !=
                   apply([](auto... vals) { return (true && ... && vals); }, feed.data());
        });

    return ret;
}

template <typename RandomEngine, size_t... Ns>
auto
boolean_test(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + __boolean_test<Ns + 2>(g, num_tests)); }

int
main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? atoi(argv[1]) : 1000000;

    int ret = 0;

    random_device         rd;
    default_random_engine g(rd());

    Indicies idxs;

    ret += test_n(g, num_tests, idxs);
    ret += boolean_test(g, num_tests, idxs);

    if(ret)
        fmt::print("{} tests failed.\n", ret);
    else
        fmt::print("All tests passed.\n");

    return ret;
}
