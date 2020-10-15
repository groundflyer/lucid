// -*- C++ -*-
// transforms.cpp -- 
#include "property_test.hpp"
#include <base/transforms.hpp>
#include <base/rng.hpp>

using namespace lucid;
using namespace std;

int
main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device         rd;
    default_random_engine g(rd());

    int ret = 0;

    auto test_property_n = [num_tests]<typename... Args>(Args && ... args)
    {
        return test_property(num_tests, 0.01, forward<Args>(args)...);
    };

    RandomDistribution<real> dist(-1000_r, 1000_r);

    auto                     argen = [&]() { return dist.template operator()<3>(g); };
    const constexpr unsigned ULP   = 5;

    ret += test_property_n(
        "translate",
        [&]() {
            return pair{Vec3{argen()}, Vec3{argen()}};
        },
        [](const auto& feed) {
            const auto& [origin, delta] = feed;
            return apply_transform_p(translate(delta), origin);
        },
        [](const auto& testing, const auto& feed) {
            const auto& [origin, delta] = feed;
            const auto origin2          = testing - delta;
            return any(!almost_equal(origin2, origin, ULP)) || !all(lucid::isfinite(origin2));
        });

    ret += test_property_n(
        "scale",
        [&]() {
            return pair{Vec3{argen()}, Vec3{argen()}};
        },
        [](const auto& feed) {
            const auto& [origin, delta] = feed;
            return apply_transform(scale(delta), origin);
        },
        [](const auto& testing, const auto& feed) {
            const auto& [origin, delta] = feed;
            const auto& origin2         = testing / delta;
            return any(!almost_equal(origin2, origin, ULP)) || !all(lucid::isfinite(origin2));
        });

    ret += test_property_n(
        "rotate",
        [&]() {
            return pair{make_normal(argen()), Pi * generate_canonical<real, 10>(g)};
        },
        [](const auto& feed) {
            const auto& [axis, angle] = feed;
            return rotate(angle, axis);
        },
        [&](const auto& testing, const auto&) {
            const Vec3 o{argen()};
            const Vec3 ot = apply_transform(testing, o);
            const real l  = length(o);
            const real lt = length(ot);
            return !(almost_equal(1_r, det(Mat3(testing)), ULP) && almost_equal(l, lt, ULP)) ||
                   all(almost_equal(o, ot, ULP)) || !all(lucid::isfinite(ot));
        });

    ret += test_property(
        num_tests,
        // fails in 7% of cases
        0.071,
        "look at",
        [&]() {
            return pair{Vec3{argen()}, Vec3{argen()}};
        },
        [&](const auto& feed) {
            const auto& [eye, target] = feed;
            return apply_transform_n(look_at(eye, target, Vec3(0_r, 1_r, 0_r)),
                                     Vec3(0_r, 0_r, 1_r));
        },
        [](const Vec3& testing, const auto& feed) {
            const auto& [eye, target] = feed;
            const Vec3 expected       = normalize(target - eye);
            return any(!almost_equal(expected, testing, ULP)) || !all(lucid::isfinite(testing));
        });

    ret += test_property_n(
        "orthonormal basis",
        [&]() { return make_normal(argen()); },
        [](const auto& feed) { return basis(feed); },
        [](const auto& testing, const auto& feed) {
            const auto& [a, b] = testing;
            const Mat3           mm{a, b, feed};
            const auto           zero = Mat3::identity() - dot(mm, transpose(mm));
            const constexpr auto ulp  = pow<sizeof(real)>(60ul);
            return any(!almost_equal(flat_ref(zero), 0_r, ulp));
        });

    if(ret)
        fmt::print("{} tests failed\n", ret);
    else
        fmt::print("All tests passed\n");

    return ret;
}
