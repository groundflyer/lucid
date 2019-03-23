// -*- C++ -*-
// primitives.cpp
#include "property_test.hpp"
#include <traversal/disk.hpp>
#include <traversal/sphere.hpp>
#include <traversal/triangle.hpp>
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

    RandomDistribution<real> big_dist(-1000_r, 1000_r);
    RandomDistribution<real> rad_dist(0.1_r, 1000_r);
    auto canonical = [&](){ return rand<real>(g); };
    auto posgen = [&](){ return Point(big_dist.template operator()<3>(g)); };
    auto normgen = [&](){ return Normal(big_dist.template operator()<3>(g)); };

    const auto property = [](const auto& sampled, const auto& prim)
                         {
                             const auto& [target, origin] = sampled;
                             const Ray tohit{origin, Normal(target - origin)};
                             const Ray tomiss{origin, Normal(origin - target)};
                             const auto hit = intersect(tohit, prim);
                             const auto miss = intersect(tomiss, prim);
                             return !hit && miss && !math::almost_equal(hit.t, distance(target, origin), 10);
                         };

    const auto test_prim = [&](auto&& name, auto&& gen, auto&& sampler)
                           { return test_property(num_tests, 0.012, name, gen, sampler, property); };

    const auto sample_prim = [&](const auto& prim){ return pair(sample(canonical, prim), posgen()); };

    ret += test_prim("Disk",
                     [&](){ return Disk(posgen(), normgen(), rad_dist(g)); },
                     sample_prim);
    ret += test_prim("Sphere",
                     [&](){ return Sphere(posgen(), rad_dist(g)); },
                     [&](const auto& prim)
                     {
                         const auto sampled_point = sample(canonical, prim);
                         const auto sign = transform([](const real val){ return std::copysign(1_r, val); },
                                                     sampled_point);
                         const auto offset = sign * prim.radius;
                         const auto origin = Vec3(rad_dist.template operator()<3>(g)) * sign + offset;
                         return pair(sampled_point, Point(origin));
                     });
    ret += test_prim("Triangle",
                     [&](){ return Triangle(posgen(), posgen(), posgen()); },
                     sample_prim);

    return ret;
}
