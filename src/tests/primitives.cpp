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
    RandomDistribution<bool> bern_dist(0.5);
    auto sgen = [&](){ return Vec2{rand<real, 2>(g)}; };
    auto posgen = [&](){ return Point(big_dist.template operator()<3>(g)); };
    auto normgen = [&](){ return Normal(big_dist.template operator()<3>(g)); };

    const auto sample_intersect_property = [](const auto& sampled, const auto& prim) noexcept
                                           {
                                               const auto& [target, origin] = sampled;
                                               const Ray tohit{origin, Normal(target - origin)};
                                               const Ray tomiss{origin, Normal(origin - target)};
                                               const auto hit = intersect(tohit, prim);
                                               const auto miss = intersect(tomiss, prim);
                                               return !hit && miss && !math::almost_equal(hit.t, distance(target, origin), 10);
                                           };

    const auto bound_property = [](const auto& testing, const auto& prim) noexcept
                                {
                                    const auto& [bound, target, origin] = testing;
                                    const Ray tohit{origin, Normal(target - origin)};
                                    const auto hit_prim = intersect(tohit, prim);
                                    const auto hit_bound = intersect(tohit, bound);
                                    return !hit_bound && hit_prim.t < hit_bound.t;
                                };

    const auto test_prim = [&](auto&& name, auto&& gen, auto&& sampler, auto&& property)
                           { return test_property(num_tests, 0.012, name, gen, sampler, property); };

    const auto intersect_test_prim = [&](auto&& name, auto&& gen, auto&& sampler)
                                     { return test_prim(name, gen, sampler, sample_intersect_property); };

    const auto bound_test_prim = [&](auto&& name, auto&& gen, auto&& sampler)
                                 { return test_prim(name, gen, sampler, bound_property); };

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

    const auto sphere_gen = [&](){ return Sphere(posgen(), rad_dist(g)); };

    const auto disk_gen = [&](){ return Disk(posgen(), normgen(), rad_dist(g)); };

    // const auto sphere_bound_gen = [&](const auto& prim)
    //                               {
    //                                   const auto [target, origin] = sample_selfoccluded_prim(prim);
    //                                   return tuple{bound(prim), target, origin};
    //                               };

    const auto bound_gen = [&](const auto& prim)
                           {
                               const AABB bbox = bound(prim);
                               const Point sp = sample(sgen(), prim);
                               const auto tb = Vector(bern_dist.template operator()<3>(g));
                               const auto offset = transform([](const auto& a, const auto& b){ return a ? b : -b; },
                                                             tb,
                                                             Vec3(rad_dist.template operator()<3>(g)));
                               const Point origin(bbox[tb] + offset);
                               return tuple(bbox, sp, origin);
                           };

    ret += intersect_test_prim("Disk: sample/trace",
                               disk_gen,
                               sample_prim);
    ret += intersect_test_prim("Sphere: sample/trace",
                               sphere_gen,
                               sample_selfoccluded_prim);
    ret += intersect_test_prim("AABB: sample/trace",
                               [&](){ return AABB(posgen(), posgen()); },
                               sample_selfoccluded_prim);
    ret += intersect_test_prim("Triangle: sample/trace",
                               [&](){ return Triangle{posgen(), posgen(), posgen()}; },
                               sample_prim);
    ret += intersect_test_prim("Quad: sample/trace",
                               [&]()
                               {
                                   const auto v00 = posgen();
                                   const auto v01 = posgen();
                                   const auto v10 = posgen();
                                   const auto v11 = v00 + v01 + v10;
                                   return Quad{v00, v01, v11, v10};
                               },
                               sample_prim);

    ret += bound_test_prim("Sphere: bound",
                           sphere_gen,
                           bound_gen);

    ret += bound_test_prim("Disk: bound",
                           disk_gen,
                           bound_gen);

    return ret;
}
