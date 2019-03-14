// -*- C++ -*-
// primitives.cpp
#include "property_test.hpp"
#include <traversal/plane.hpp>
#include <traversal/disk.hpp>
#include <base/rng.hpp>

using namespace std;
using namespace yapt;

int main(int argc, char *argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    int ret = 0;

    RandomDistribution<real> pos_dist(-1000_r, 1000_r);
    RandomDistribution<real> rad_dist(0.1_r, 1000_r);

    auto argen = [&](){ return pos_dist.template operator()<3>(g); };

    const auto sampler = [&](const auto& prim){ return pair{ sample([&](){ return pos_dist(g); }, prim), Point(argen()) }; };

    const auto property = [](const auto& testing, const auto& prim)
                         {
                             const auto& [target, origin] = testing;
                             const Ray tohit{origin, Normal(target - origin)};
                             const Ray tomiss{origin, Normal(origin - target)};
                             const auto hit = intersect(tohit, prim);
                             const auto miss = intersect(tomiss, prim);
                             return !hit && miss && !math::almost_equal(hit.t, distance(target, origin), 10);
                         };

    const auto test_prim = [&](auto&& name, auto&& gen)
                           { return test_property(num_tests, 0.01, name, gen, sampler, property); };


    ret += test_prim("Plane", [&](){ return Plane(Point(argen()), Normal(argen())); });
    ret += test_prim("Disk", [&](){ return Disk(Point(argen()), Normal(argen()), rad_dist(g)); });

    return ret;
}
