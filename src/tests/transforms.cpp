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

    init_log();

    int ret = 0;

    auto test_property_n = [num_tests](auto&& ... args)
                           { return test_property(num_tests, forward<decltype(args)>(args)...); };

    RandomDistribution<real> dist(-1000_r, 1000_r);

    auto argen = [&](){ return dist.template operator()<3>(g); };

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
                               return any(!almost_equal(origin2, origin));
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
                               return any(!almost_equal(origin2, origin));
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
                               return !(math::almost_equal(1_r, det(Mat3(testing)), 10) && math::almost_equal(l, lt, 10)) || all(almost_equal(o, ot, 10));
                           });

    ret += test_property_n("look at",
                           [&](){ return pair{Point{argen()}, Point{argen()}}; },
                           [&](const auto& feed)
                           {
                               const auto& [eye, target] = feed;
                               return apply_transform(look_at(eye, target), Normal(0_r, 0_r, 1_r));
                           },
                           [](const auto& testing, const auto& feed)
                           {
                               const auto& [eye, target] = feed;
                               const auto expected = Normal(target - eye);
                               // TODO: in some cases testing is exactly opposite to exptected; investigate this
                               return any(!almost_equal(abs(expected), abs(testing), 10));
                           });

    return ret;
}
