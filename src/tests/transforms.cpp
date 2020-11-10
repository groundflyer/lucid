// -*- C++ -*-
// transforms.cpp --
#include "property_test.hpp"
#include <base/transforms.hpp>
#include <utils/tuple.hpp>

#include <random>

using namespace lucid;
using namespace std;

int
main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device         rd;
    default_random_engine g(rd());

    int ret = 0;

    auto test_property_n = [num_tests]<typename... Args>(Args && ... args) noexcept
    {
        return test_property(num_tests, 0.01, forward<Args>(args)...);
    };

    std::uniform_real_distribution<real> dist(-1000_r, 1000_r);

    auto argen  = [&]() noexcept { return generate<3>(dist, g); };
    auto mkpair = [&]() noexcept { return pair{Vec3{argen()}, Vec3{argen()}}; };
    static const constexpr unsigned ULP = 5;

    ret += test_property_n(
        "translate",
        mkpair,
        [](const Vec3& origin, const Vec3& delta) noexcept {
            return apply_transform_p(translate(delta), origin);
        },
        [](const Vec3& testing, const auto& feed) noexcept {
            const auto& [origin, delta] = feed;
            const auto origin2          = testing - delta;
            return any(!almost_equal(origin2, origin, ULP)) || !all(lucid::isfinite(origin2));
        });

    ret += test_property_n(
        "scale",
        mkpair,
        [](const Vec3& origin, const Vec3& delta) noexcept {
            return apply_transform(scale(delta), origin);
        },
        [](const Vec3& testing, const auto& feed) noexcept {
            const auto& [origin, delta] = feed;
            const auto& origin2         = testing / delta;
            return any(!almost_equal(origin2, origin, ULP)) || !all(lucid::isfinite(origin2));
        });

    ret += test_property_n(
        "rotate",
        [&]() noexcept {
            return pair{Pi * generate_canonical<real, 10>(g), make_normal(argen())};
        },
        lucid::rotate,
        [&](const Mat4& testing, const auto&) noexcept {
            const Vec3 o{argen()};
            const Vec3 ot = apply_transform(testing, o);
            const real l  = length(o);
            const real lt = length(ot);
            return !(almost_equal(1_r, det(Mat3(testing)), ULP) && almost_equal(l, lt, ULP)) ||
                   all(almost_equal(o, ot, ULP)) || !all(lucid::isfinite(ot));
        });

    ret += test_property(
        num_tests,
        // fails in 7% of cases with clang
        0.071,
        "look at",
        mkpair,
        [&](const Vec3& eye, const Vec3& target) noexcept {
            return apply_transform_n(look_at(eye, target, Vec3(0_r, 1_r, 0_r)),
                                     Vec3(0_r, 0_r, 1_r));
        },
        [](const Vec3& testing, const auto& feed) noexcept {
            const auto& [eye, target] = feed;
            const Vec3 expected       = normalize(target - eye);
            return any(!almost_equal(expected, testing, ULP)) || !all(lucid::isfinite(testing));
        });

    ret += test_property_n(
        "orthonormal basis",
        [&]() noexcept { return make_normal(argen()); },
        basis,
        [](const auto& testing, const auto& feed) noexcept {
            const auto& [a, b] = testing;
            const Mat3             mm{a, b, feed};
            const Mat3             zero = Mat3::identity() - mm * transpose(mm);
            const constexpr size_t ulp  = static_pow<sizeof(real)>(60ul);
            return any(!almost_equal(flat_ref(zero), 0_r, ulp));
        });

    if(ret)
        fmt::print("{} tests failed\n", ret);
    else
        fmt::print("All tests passed\n");

    return ret;
}
