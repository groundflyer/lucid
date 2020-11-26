// -*- C++ -*-
/// @file
/// Vector tests.

#include "property_test.hpp"

#include <base/vector.hpp>
#include <utils/tuple.hpp>
#include <utils/typeinfo.hpp>
#include <utils/typelist.hpp>

#include <cstdlib>
#include <random>

using namespace std;
using namespace lucid;

static const constexpr size_t MaxN = 4;
using Indicies                     = make_index_sequence<MaxN - 1>;
using ArithmeticTypes              = typelist<int, long, float, double>;

template <size_t I, typename A, typename B>
constexpr bool
not_equal(A&& a, B&& b) noexcept
{
    return std::get<I>(std::forward<A>(a)) != std::get<I>(std::forward<B>(b));
}

template <typename A, typename B, size_t... I>
constexpr bool
assert_tuples(A&& a, B&& b, std::index_sequence<I...>) noexcept
{
    return (false || ... || not_equal<I>(std::forward<A>(a), std::forward<B>(b)));
}

/// @brief Test lucid::Vector.
/// @tparam T Value type.
/// @tparam N Dimensionality.
template <typename T, size_t N, typename RandomEngine>
unsigned
vector_test(RandomEngine& g, const size_t num_tests) noexcept
{
    using Dist                = std::conditional_t<std::is_integral_v<T>,
                                    std::uniform_int_distribution<T>,
                                    std::uniform_real_distribution<T>>;
    using Vec                 = Vector<T, N, array>;
    const auto t_typestring   = get_typeinfo_string(T{});
    const auto vec_typestring = get_typeinfo_string(Vec{});
    const auto arr_typestring = get_typeinfo_string(Vec().data());
    Dist       dist(T{-10000}, T{10000});
    unsigned   ret = 0;

    const constexpr double threshold       = is_floating_point_v<T> ? 0.01 : 0.0;
    const auto             test_property_n = [num_tests, threshold](auto&&... args) {
        return test_property(num_tests, threshold, forward<decltype(args)>(args)...);
    };

    constexpr auto vmaker = maker<Vec>;

    /// @test Construct @p Vector with a single scalar.
    ///
    /// @f$\mathrm{V}(a) = \mathrm{V}(a, a, a\dots)@f$
    ret += test_property_n(
        "{}({})"_format(vec_typestring, t_typestring),
        [&]() noexcept { return dist(g); },
        vmaker,
        [](const auto& testing, const auto& feed) noexcept { return any(testing != feed); });

    // array generator
    auto argen = [&]() noexcept { return generate<N>(dist, g); };

    // array assertion
    const auto arass = [](const Vec& testing, const array<T, N>& feed) noexcept {
        return assert_tuples(testing, feed, make_index_sequence<N>{}) &&
               !all(lucid::isfinite(testing));
    };

    /// @test Standard vector constructor.
    ///
    /// @f$\mathrm{V}(a,b,c\dots) = \mathrm{V}(a, b, c\dots)@f$
    ret += test_property_n(
        "{}({})"_format(vec_typestring, arr_typestring), argen, tuple_maker<Vec>, arass);

    /// @test Array vector constructor.
    ///
    /// @f$\mathrm{V}([a,b,c\dots]) = \mathrm{V}(a, b, c\dots)@f$
    ret += test_property_n("{}({})"_format(vec_typestring, arr_typestring), argen, vmaker, arass);

    if constexpr(N > 2)
    {
        const constexpr std::size_t N1 = N - 1;
        /// @test Construct vector from scalar and smaller vector.
        ///
        /// @f$\mathrm{V}_N(a, \mathrm{V}_{N-1}(b, c\dots)) = \mathrm{V}_{N}(a, b, c\dots)@f$
        ret += test_property_n(
            "{0}({1}, Vector<{1}, {2}>)"_format(vec_typestring, t_typestring, N1),
            [&]() noexcept { return pair(dist(g), Vector<T, N1, array>(generate<N1>(dist, g))); },
            vmaker,
            [](const Vec& testing, const auto& feed) noexcept {
                const auto& [v0, vv] = feed;
                bool ret             = testing[0] != v0;
                for(size_t i = 0; i < N1; ++i) ret |= vv[i] != testing[i + 1];
                return ret || !all(lucid::isfinite(testing));
            });
    }

    auto vgen = [&]() noexcept { return Vec(argen()); };

    // vector and scalar value generator
    auto vsgen = [&]() noexcept { return pair(vgen(), dist(g)); };

    const auto assertion = [](const auto& a, const auto& b) noexcept {
        if constexpr(is_floating_point_v<T>)
        {
            return any(!almost_equal(a, b, 200)) || !all(lucid::isfinite(a)) ||
                   !all(lucid::isfinite(b));
        }
        else
            return any(a != b);
    };

    /// @test Vector-scalar addition and subtraction.
    ///
    /// @f$\mathbf{v} + s = \mathbf{v} + s - s@f$
    ret += test_property_n("{} +- {}"_format(vec_typestring, t_typestring),
                           vsgen,
                           lucid::plus,
                           [&](const auto& testing, const auto feed) noexcept {
                               const auto& [vec, val] = feed;
                               return assertion(testing - val, vec);
                           });

    /// @test Inplace vector-scalar addition and subtraction.
    ///
    /// @f$\mathbf{v} + s = \mathbf{v} + s - s@f$
    ret += test_property_n(
        "{} +-= {}"_format(vec_typestring, t_typestring),
        vsgen,
        [](Vec vec, const T& val) noexcept {
            vec += val;
            return vec;
        },
        [&](auto testing, const auto& feed) noexcept {
            auto [vec, val] = feed;
            testing -= val;
            return assertion(testing, vec);
        });

    // distribution to generate divizor that is guaranteed to be greater than zero
    Dist divdist(T{1}, T{10000});

    // random sign generator
    auto signgen = [&, bdist = std::bernoulli_distribution(0.5)]() mutable noexcept {
        return static_cast<T>(minus_one_pow(bdist(g)));
    };

    // vector and divisor generator
    auto vdgen = [&]() noexcept { return pair(vgen(), divdist(g) * signgen()); };

    /// @test Vector-scalar multiplication and division.
    ///
    /// @f$\mathbf{v} s = \frac{\mathbf{v} s}{s}@f$.
    ret += test_property_n("{} */ {}"_format(vec_typestring, t_typestring),
                           vdgen,
                           lucid::multiplies,
                           [&](const auto& testing, const auto& feed) noexcept {
                               const auto& [vec, val] = feed;
                               return assertion(testing / val, vec);
                           });

    /// @test Inplace vector-scalar multiplication and division.
    ///
    /// @f$\mathbf{v} s = \frac{\mathbf{v} s}{s}@f$
    ret += test_property_n(
        "{} */= {}"_format(vec_typestring, t_typestring),
        vdgen,
        [](Vec vec, const T& val) noexcept {
            vec *= val;
            return vec;
        },
        [&](auto testing, const auto& feed) noexcept {
            const auto& [vec, val] = feed;
            testing /= val;
            return assertion(testing, vec);
        });

    // pair of vectors generator
    auto vvgen = [&]() noexcept { return pair(vgen(), vgen()); };

    /// @test Vector-vector addition and subtraction.
    ///
    /// @f$\mathbf{v}_0 + \mathbf{v}_1 = \mathbf{v}_0 + \mathbf{v}_1 - \mathbf{v}_1@f$
    ret += test_property_n("{0} +- {0}"_format(vec_typestring),
                           vvgen,
                           lucid::plus,
                           [&](const auto& testing, const auto& feed) noexcept {
                               const auto& [vec1, vec2] = feed;
                               return assertion(testing - vec2, vec1);
                           });

    /// @test Inplace vector-vector addition and subtraction.
    ///
    /// @f$\mathbf{v}_0 + \mathbf{v}_1 = \mathbf{v}_0 + \mathbf{v}_1 -
    /// \mathbf{v}_1@f$
    ret += test_property_n(
        "{0} +-= {0}"_format(vec_typestring),
        vvgen,
        [](Vec vec1, const Vec& vec2) noexcept {
            vec1 += vec2;
            return vec1;
        },
        [&](auto testing, const auto& feed) noexcept {
            auto [vec1, vec2] = feed;
            testing -= vec2;
            return assertion(testing, vec1);
        });

    /// @test Vector minmax.
    ///
    /// @f$\mathrm{min}(\mathbf{v}_0, \mathbf{v}_1) < \mathrm{max}(\mathbf{v}_0,
    /// \mathbf{v}_1)@f$
    ret += test_property_n("min({0}, {0}), max({0}, {0})"_format(vec_typestring),
                           vvgen,
                           lucid::minmax,
                           // [](const auto& feed) noexcept {
                           //     const auto& [vec1, vec2] = feed;
                           //     return pair{lucid::min(vec1, vec2), lucid::max(vec1, vec2)};
                           // },
                           [&](const auto& testing, const auto&) noexcept {
                               const auto& [vmin, vmax] = testing;
                               return any(vmin > vmax) || !all(lucid::isfinite(vmin)) ||
                                      !all(lucid::isfinite(vmax));
                           });

    // vector and vector-divizor generator
    auto vvdgen = [&]() noexcept { return pair(vgen(), Vec(generate<N>(divdist, g)) * signgen()); };

    /// @test Vector-vector multiplication and division.
    ///
    /// @f$\mathbf{v}_0 \mathbf{v}_1 = \frac{\mathbf{v}_0 \mathbf{v}_1}{\mathbf{v}_1}@f$
    ret += test_property_n("{0} */ {0}"_format(vec_typestring),
                           vvdgen,
                           lucid::multiplies,
                           [&](const auto& testing, const auto& feed) noexcept {
                               const auto& [vec1, vec2] = feed;
                               return assertion(testing / vec2, vec1);
                           });

    /// @test Inplace vector-vector multiplication and division.
    ///
    /// @f$\mathbf{v}_0 \mathbf{v}_1 = \frac{\mathbf{v}_0
    /// \mathbf{v}_1}{\mathbf{v}_1}@f$
    ret += test_property_n(
        "{0} */= {0}"_format(vec_typestring),
        vvdgen,
        [](Vec vec1, const Vec& vec2) noexcept {
            vec1 *= vec2;
            return vec1;
        },
        [&](auto testing, const auto& feed) noexcept {
            const auto& [vec1, vec2] = feed;
            testing /= vec2;
            return assertion(testing, vec1);
        });

    if constexpr(is_floating_point_v<T>)
    {
        /// @test Check if dot product of the same vector is equal its length.
        ///
        /// @f$\mathbf{v} \cdot \mathbf{v} = \mathrm{length}(\mathbf{v})@f$
        ret += test_property_n(
            "{}: A dot A = length A"_format(vec_typestring),
            vgen,
            [](const Vec& feed) noexcept { return dot(feed, feed); },
            [](const T testing, const Vec& feed) noexcept {
                return !almost_equal(lucid::sqrt(testing), length(feed), 5) ||
                       !std::isfinite(testing);
            });

        /// @test Check if length of normalized vector is equal to one.
        ///
        /// @f$\mathrm{length}(\mathrm{normalize}(\mathbf{v})) = 1@f$
        ret += test_property_n("normalize({}) = 1"_format(vec_typestring),
                               vgen,
                               normalize,
                               [](const Vec& testing, Vec) noexcept {
                                   return !almost_equal(length(testing), T{1}, 5) ||
                                          !all(lucid::isfinite(testing));
                               });

        if constexpr(N > 2)
            /// @test Check if cross product of two vectors is perpendicular vector.
            ///
            /// @f$(\mathbf{v}_0 \times \mathbf{v}_1) \bot \mathbf{v}_0 \bot \mathbf{v}_1@f$
            ret += test_property(
                num_tests,
                0.01,
                "{}: C <- cross A B | A dot C = B dot C = 0"_format(vec_typestring),
                [&]() noexcept { return pair(normalize(vgen()), normalize(vgen())); },
                cross,
                [](const Vec& testing, const auto& feed) noexcept {
                    const auto& [a, b]      = feed;
                    const T              at = dot(a, testing);
                    const T              bt = dot(b, testing);
                    const constexpr auto ulp =
                        static_pow<sizeof(T)>(is_same_v<T, double> ? 100ul : 55u);
                    return !(almost_equal(at, T{0}, ulp) || almost_equal(bt, T{0}, ulp)) ||
                           !all(lucid::isfinite(testing));
                });
    }

    // structure binding test
    if constexpr(N == 4)
    {
        const Vec vec            = vgen();
        const auto& [x, y, z, w] = vec;
        const auto asrt          = &x != &std::get<0>(vec) || &y != &std::get<1>(vec) ||
                          &z != &std::get<2>(vec) || &w != &std::get<3>(vec);

        ret += asrt;
    }

    return ret;
}

template <size_t N, typename RandomEngine, typename... Ts>
unsigned
test_t(RandomEngine& g, const size_t num_tests, typelist<Ts...>) noexcept
{
    return (0u + ... + vector_test<Ts, N>(g, num_tests));
}

template <typename RandomEngine, size_t... Ns>
unsigned
test_n(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{
    return (0u + ... + test_t<Ns + 2>(g, num_tests, ArithmeticTypes{}));
}

/// @brief Test lucid::Vector with boolean value type.
/// @tparam Vector dimensionality.
template <size_t N, typename RandomEngine>
unsigned
vector_bool_test(RandomEngine& g, const size_t num_tests) noexcept
{
    std::bernoulli_distribution dist(0.5);
    unsigned                    ret            = 0;
    auto                        vec_typestring = get_typeinfo_string(Vector<bool, N, array>{});

    static const constexpr double threshold       = 0.0;
    auto                          test_property_n = [num_tests](auto&&... args) {
        return test_property(num_tests, threshold, forward<decltype(args)>(args)...);
    };

    /// @test @f$\mathrm{any}(\mathbf{v}(a,b,c\dots)) = a \vee b \vee \dots \vee c@f$
    ret += test_property_n(
        "any({})"_format(vec_typestring),
        [&]() noexcept { return Vector(generate<N>(dist, g)); },
        [](const Vector<bool, N, array>& feed) noexcept { return any(feed); },
        [](const bool testing, const Vector<bool, N, array>& feed) noexcept {
            return testing !=
                   apply([](auto... vals) { return (false || ... || vals); }, feed.data());
        });

    /// @test @f$\mathrm{all}(\mathbf{v}(a,b,c\dots)) = a \wedge b \wedge \dots \wedge c@f$
    ret += test_property_n(
        "all({})"_format(vec_typestring),
        [&]() noexcept { return Vector(generate<N>(dist, g)); },
        [](const Vector<bool, N, array>& feed) noexcept { return all(feed); },
        [](const bool testing, const Vector<bool, N, array>& feed) noexcept {
            return testing !=
                   apply([](auto... vals) { return (true && ... && vals); }, feed.data());
        });

    return ret;
}

template <typename RandomEngine, size_t... Ns>
unsigned
vector_bool_test(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{
    return (0u + ... + vector_bool_test<Ns + 2>(g, num_tests));
}

int
main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? atoi(argv[1]) : 1000000;

    int ret = 0;

    random_device         rd;
    default_random_engine g(rd());

    Indicies idxs;

    ret += test_n(g, num_tests, idxs);
    ret += vector_bool_test(g, num_tests, idxs);

    if(ret)
        fmt::print("{} tests failed.\n", ret);
    else
        fmt::print("All tests passed.\n");

    return ret;
}
