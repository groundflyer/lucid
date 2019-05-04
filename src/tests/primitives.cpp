// -*- C++ -*-
// primitives.cpp
#include "property_test.hpp"
#include <primitives/aabb.hpp>
#include <primitives/disk.hpp>
#include <primitives/sphere.hpp>
#include <primitives/triangle.hpp>
#include <primitives/quad.hpp>
#include <primitives/generic.hpp>
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
                                               return !hit && miss && !almost_equal(hit.t, distance(target, origin), 10);
                                           };

    const auto bound_property = [](const auto& testing, const auto& prim) noexcept
                                {
                                    const auto& [bound, ray] = testing;
                                    const auto hit_prim = intersect(ray, prim);
                                    const auto hit_bound = intersect(ray, bound);
                                    return !hit_bound && hit_prim && hit_prim.t < hit_bound.t;
                                };

    const auto test_prim = [&](auto&& name, auto&& gen, auto&& sampler, auto&& property)
                           { return test_property(num_tests, 0.022, name, gen, sampler, property); };

    const auto intersect_test_prim = [&](auto&& name, auto&& gen, auto&& sampler)
                                     { return test_prim(name, gen, sampler, sample_intersect_property); };

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

    const auto bound_gen = [&](const auto& prim)
                           {
                               const AABB bbox = bound(prim);
                               const Point sp = sample(sgen(), prim);
                               const auto tb = Vector(bern_dist.template operator()<3>(g));
                               const auto offset = transform([](const auto& a, const auto& b){ return a ? b : -b; },
                                                             tb,
                                                             Vec3(rad_dist.template operator()<3>(g)));
                               const Point origin(bbox[tb] + offset);
                               return pair{bbox, Ray(origin, Normal(sp - origin))};
                           };

    const auto bound_test_prim = [&](auto&& name, auto&& gen)
                                 { return test_prim(name, gen, bound_gen, bound_property); };

    const auto aabb_gen = [&](){ return AABB(posgen(), posgen()); };

    const auto sphere_gen = [&](){ return Sphere(posgen(), rad_dist(g)); };

    const auto disk_gen = [&](){ return Disk(posgen(), normgen(), rad_dist(g)); };

    const auto triangle_gen = [&](){ return Triangle{posgen(), posgen(), posgen()}; };

    const auto quad_gen = [&]()
                          {
                              const auto v00 = posgen();
                              const auto v01 = posgen();
                              const auto v10 = posgen();
                              const auto v11 = v00 + v01 + v10;
                              return Quad{v00, v01, v11, v10};
                          };

    const auto all_gens =
        tuple{[&]() { return GenericPrimitive(sphere_gen()); },
              [&]() { return GenericPrimitive(triangle_gen()); },
              [&]() { return GenericPrimitive(quad_gen()); },
              [&]() { return GenericPrimitive(disk_gen()); }};

    RandomDistribution<size_t> prim_choose(0, std::tuple_size_v<decltype(all_gens)>);

    const auto rand_prim_gen = [&]()
                                  { return switcher(prim_choose(g), all_gens); };

    ret += intersect_test_prim("Disk: sample/trace",
                               disk_gen,
                               sample_prim);
    ret += intersect_test_prim("Sphere: sample/trace",
                               sphere_gen,
                               sample_selfoccluded_prim);
    ret += intersect_test_prim("AABB: sample/trace",
                               aabb_gen,
                               sample_selfoccluded_prim);
    ret += intersect_test_prim("Triangle: sample/trace",
                               triangle_gen,
                               sample_prim);
    ret += intersect_test_prim("Quad: sample/trace",
                               quad_gen,
                               sample_prim);

    ret += bound_test_prim("Sphere: bound", sphere_gen);

    ret += bound_test_prim("Disk: bound", disk_gen);

    ret += bound_test_prim("Triangle: bound", triangle_gen);

    ret += bound_test_prim("Quad: bound", quad_gen);

    ret += bound_test_prim("GenericPrimitive: bound", rand_prim_gen);

    if(ret)
        fmt::print("{} tests failed\n", ret);
    else
        fmt::print("All tests passed\n");

    return ret;
}
