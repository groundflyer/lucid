// -*- C++ -*-
// primitives.cpp
#include "property_test.hpp"
#include <primitives/aabb.hpp>
#include <primitives/disk.hpp>
#include <primitives/sphere.hpp>
#include <primitives/triangle.hpp>
#include <primitives/quad.hpp>
#include <base/rng.hpp>

#include <utils/seq.hpp>

using namespace std;
using namespace yapt;

constexpr auto
radius(const Sphere& prim) noexcept
{ return prim.radius; }

constexpr auto
radius(const AABB& prim) noexcept
{ return distance(centroid(prim), prim.vmax); }

int main(int argc, char *argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    int ret = 0;

    RandomDistribution<real> big_dist(-1000_r, 1000_r);
    RandomDistribution<real> rad_dist(0.1_r, 1000_r);
    auto sgen = [&](){ return Vec2{rand<real, 2>(g)}; };
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

    const auto sample_prim = [&](const auto& prim){ return pair(sample(sgen(), prim), posgen()); };

    const auto sample_selfoccluded_prim = [&](const auto& prim)
                                          {
                                              const auto sampled_point = sample(sgen(), prim);
                                              const auto offset = sampled_point - centroid(prim);
                                              const auto sign = transform([](const real val){ return std::copysign(1_r, val); },
                                                                          offset);
                                              const auto origin = sampled_point + Vec3(rad_dist.template operator()<3>(g)) * sign * radius(prim) + offset;
                                              return pair(sampled_point, Point(origin));
                                          };

    ret += test_prim("Disk: sample/trace",
                     [&](){ return Disk(posgen(), normgen(), rad_dist(g)); },
                     sample_prim);
    ret += test_prim("Sphere: sample/trace",
                     [&](){ return Sphere(posgen(), rad_dist(g)); },
                     sample_selfoccluded_prim);
    ret += test_prim("AABB: sample/trace",
                     [&](){ return AABB(posgen(), posgen()); },
                     sample_selfoccluded_prim);
    ret += test_prim("Triangle: sample/trace",
                     [&](){ return Triangle{posgen(), posgen(), posgen()}; },
                     sample_prim);
    ret += test_prim("Quad: sample/trace",
                     [&]()
                     {
                         const auto v00 = posgen();
                         const auto v01 = posgen();
                         const auto v10 = posgen();
                         const auto v11 = v00 + v01 + v10;
                         return Quad{v00, v01, v11, v10};
                     },
                     sample_prim);

    return ret;
}
