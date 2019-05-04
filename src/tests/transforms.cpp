// -*- C++ -*-
// transforms.cpp -- 
#include "property_test.hpp"
#include <base/transforms.hpp>
#include <base/rng.hpp>

using namespace yapt;
using namespace std;

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device rd;
    default_random_engine g(rd());

    int ret = 0;

    auto test_property_n = [num_tests](auto&& ... args)
                           { return test_property(num_tests, 0.01, forward<decltype(args)>(args)...); };

    RandomDistribution<real> dist(-1000_r, 1000_r);

    auto argen = [&](){ return dist.template operator()<3>(g); };
    const constexpr unsigned ULP = 5;

    ret += test_property_n("translate",
                           [&](){ return pair{Point{argen()}, Vec3{argen()}}; },
                           [](const auto& feed)
                           {
                               const auto& [origin, delta] = feed;
                               return apply_transform(translate(delta), origin);
                           },
                           [](const auto& testing, const auto& feed)
                           {
                               const auto& [origin, delta] = feed;
                               const auto origin2 = testing - delta;
                               return any(!almost_equal(origin2, origin, ULP));
                           });

    ret += test_property_n("scale",
                           [&](){ return pair{Point{argen()}, Vec3{argen()}}; },
                           [](const auto& feed)
                           {
                               const auto& [origin, delta] = feed;
                               return apply_transform(scale(delta), origin);
                           },
                           [](const auto& testing, const auto& feed)
                           {
                               const auto& [origin, delta] = feed;
                               const auto& origin2 = testing / delta;
                               return any(!almost_equal(origin2, origin, ULP));
                           });

    ret += test_property_n("rotate",
                           [&](){ return pair{Normal{argen()}, math::PI<real> * generate_canonical<real, 10>(g)}; },
                           [](const auto& feed)
                           {
                               const auto& [axis, angle] = feed;
                               return rotate(angle, axis);
                           },
                           [&](const auto& testing, const auto&)
                           {
                               const Vec3 o{argen()};
                               const auto ot = apply_transform(testing, o);
                               const auto l = length(o);
                               const auto lt = length(ot);
                               return !(almost_equal(1_r, det(Mat3(testing)), ULP) && almost_equal(l, lt, ULP)) || all(almost_equal(o, ot, ULP));
                           });

    ret += test_property(num_tests,
                         0.071,
                         "look at",
                         [&](){ return pair{Point{argen()}, Point{argen()}}; },
                         [&](const auto& feed)
                         {
                             const auto& [eye, target] = feed;
                             return apply_transform(look_at(eye, target, Normal(0_r, 1_r, 0_r)), Normal(0_r, 0_r, 1_r));
                         },
                         [](const auto& testing, const auto& feed)
                         {
                             const auto& [eye, target] = feed;
                             const auto expected = Normal(target - eye);
                             // TODO: in some cases testing is exactly opposite to exptected; investigate this
                             return any(!almost_equal(expected, testing, ULP));
                         });

    ret += test_property_n("orthonormal basis",
                           [&](){ return Normal(argen()); },
                           [](const auto& feed){ return basis(feed); },
                           [](const auto& testing, const auto& feed)
                           {
                               const auto& [a, b] = testing;
                               const Mat3 mm{a, b, feed};
                               const auto zero = Mat3::identity() - mm.dot(transpose(mm));
                               const constexpr auto ulp = pow<sizeof(real)>(60ul);
                               return any(!almost_equal(zero.flat_ref(), 0_r, ulp));
                           });

    if(ret)
        fmt::print("{} tests failed\n", ret);
    else
        fmt::print("All tests passed\n");

    return ret;
}
