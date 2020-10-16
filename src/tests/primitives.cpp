// -*- C++ -*-
// primitives.cpp
#include "property_test.hpp"
#include <base/rng.hpp>
#include <primitives/aabb.hpp>
#include <primitives/disk.hpp>
#include <primitives/generic.hpp>
#include <primitives/quad.hpp>
#include <primitives/sphere.hpp>
#include <primitives/triangle.hpp>

#include <utils/tuple.hpp>

using namespace std;
using namespace lucid;

constexpr auto
radius(const Sphere& prim) noexcept
{
    return prim.radius;
}

constexpr auto
radius(const AABB& prim) noexcept
{ return distance(centroid(prim), prim.vmax); }

int
main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    random_device         rd;
    default_random_engine g(rd());

    int ret = 0;

    static const real        val_range = 100;
    RandomDistribution<real> big_dist(-val_range, val_range);
    RandomDistribution<real> rad_dist(0.1_r, val_range);
    RandomDistribution<bool> bern_dist(0.5);
    auto                     sgen   = [&]() { return Vec2{rand<real, 2>(g)}; };
    auto                     posgen = [&]() { return Vec3(big_dist.template operator()<3>(g)); };
    auto normgen = [&]() { return make_normal(big_dist.template operator()<3>(g)); };

    const auto sample_intersect_property = [](const auto& sampled, const auto& prim) noexcept {
        const auto& [target, origin] = sampled;
        const Ray  tohit{origin, target - origin};
        const Ray  tomiss{origin, origin - target};
        const auto hit  = intersect(tohit, prim);
        const auto miss = intersect(tomiss, prim);
        return !hit || miss || !almost_equal(hit.t, distance(target, origin), 10);
    };

    const auto bound_property = [](const auto& testing, const auto& prim) noexcept {
        const auto& [bound, ray] = testing;
        const auto hit_prim      = intersect(ray, prim);
        const auto hit_bound     = intersect(ray, bound);
        return !hit_bound || hit_prim.t < hit_bound.t;
    };

    const auto test_prim = [&](auto&& name, auto&& gen, auto&& sampler, auto&& property) {
        return test_property(num_tests, 0.05, name, gen, sampler, property);
    };

    const auto intersect_test_prim = [&](auto&& name, auto&& gen, auto&& sampler) {
        return test_prim(name, gen, sampler, sample_intersect_property);
    };

    const auto sample_prim = [&](const auto& prim) { return pair(sample(sgen(), prim), posgen()); };

    const auto sample_selfoccluded_prim = [&](const auto& prim) noexcept {
        const auto sampled_point = sample(sgen(), prim);
        const auto offset        = sampled_point - centroid(prim);
        const auto sign = transform([](const real val) { return std::copysign(1_r, val); }, offset);
        const auto origin =
            sampled_point + Vec3(rad_dist.template operator()<3>(g)) * sign + offset;
        return pair(sampled_point, Vec3(origin));
    };

    const auto bound_gen = [&](const auto& prim) noexcept {
        const AABB bbox   = bound(prim);
        const Vec3 sp     = sample(sgen(), prim);
        const auto tb     = Vector(bern_dist.template operator()<3>(g));
        const auto offset = transform([](const auto& a, const auto& b) { return a ? b : -b; },
                                      tb,
                                      Vec3(rad_dist.template operator()<3>(g)));
        const Vec3 origin(bbox[tb] + offset);
        return pair{bbox, Ray(origin, sp - origin)};
    };

    const auto bound_test_prim = [&](auto&& name, auto&& gen) noexcept {
        return test_prim(name, gen, bound_gen, bound_property);
    };

    const auto aabb_gen     = [&]() noexcept { return AABB(posgen(), posgen()); };
    const auto sphere_gen   = [&]() noexcept { return Sphere(posgen(), rad_dist(g)); };
    const auto disk_gen     = [&]() noexcept { return Disk(posgen(), normgen(), rad_dist(g)); };
    const auto triangle_gen = [&]() noexcept { return Triangle{posgen(), posgen(), posgen()}; };
    const auto quad_gen     = [&]() noexcept {
        const Vec3 v00{0, 0, 0};
        const Vec3 v01{0, rad_dist(g), 0};
        const Vec3 v10{rad_dist(g), 0, 0};
        const Vec3 v11 = v00 + v01 + v10;
        const auto t   = dot(rotate(big_dist(g), normgen()), translate(posgen()));
        return apply_transform(t, Quad{v00, v01, v11, v10});
    };

    const auto aabb_gen_l = [&](const real max_rad, const Vec3&) noexcept {
        return AABB(-Vec3(rand<real, 3>(g)) * max_rad * 0.5_r,
                    Vec3(rand<real, 3>(g)) * max_rad * 0.5_r);
    };
    const auto sphere_gen_l = [&](const real max_rad, const Vec3&) noexcept {
        return Sphere(Vec3(0), rand<real>(g) * max_rad);
    };
    const auto disk_gen_l = [&](const real, const Vec3& n) noexcept {
        return Disk(Vec3(0), n, rad_dist(g));
    };
    const auto triangle_gen_l = [&](const real, const Vec3& n) noexcept {
        const Vec3 v0{-rad_dist(g), -rad_dist(g), 0};
        const Vec3 v1{0, rad_dist(g), 0};
        const Vec3 v2{rad_dist(g), -rad_dist(g), 0};
        Mat4       t(basis_matrix(n));
        t[3][3] = 1;
        return apply_transform(t, Triangle{v0, v1, v2});
    };
    const auto quad_gen_l = [&](const real, const Vec3& n) noexcept {
        const real x = rad_dist(g);
        const real y = rad_dist(g);
        const Vec3 v00{-x, -y, 0};
        const Vec3 v01{0, y * 2, 0};
        const Vec3 v10{x * 2, 0, 0};
        const auto v11 = v00 + v01 + v10;
        Mat4       t(basis_matrix(n));
        t[3][3] = 1;
        return apply_transform(t, Quad{v00, v01, v11, v10});
    };

    const auto gp_gen   = tuple{sphere_gen, triangle_gen, quad_gen, disk_gen};
    const auto gp_gen_l = tuple{sphere_gen_l, triangle_gen_l, quad_gen_l, disk_gen_l, aabb_gen_l};

    RandomDistribution<size_t> gp_choose(0ul, std::tuple_size_v<decltype(gp_gen)> - 1);
    RandomDistribution<size_t> gp_choose_l(0ul, std::tuple_size_v<decltype(gp_gen_l)> - 1);

    const auto rand_prim_gen   = [&]() noexcept { return switcher_func(gp_choose(g), gp_gen); };
    const auto rand_prim_gen_l = [&](const real m, const Vec3& n) noexcept {
        return switcher_func(gp_choose_l(g), gp_gen_l, m, n);
    };

    ret += intersect_test_prim("Disk: sample/trace", disk_gen, sample_prim);
    ret += intersect_test_prim("Sphere: sample/trace", sphere_gen, sample_selfoccluded_prim);
    ret += intersect_test_prim("AABB: sample/trace", aabb_gen, sample_selfoccluded_prim);
    ret += intersect_test_prim("Triangle: sample/trace", triangle_gen, sample_prim);
    ret += intersect_test_prim("Quad: sample/trace", quad_gen, sample_prim);

    ret += bound_test_prim("Sphere: bound", sphere_gen);

    ret += bound_test_prim("Disk: bound", disk_gen);

    ret += bound_test_prim("Triangle: bound", triangle_gen);

    ret += bound_test_prim("Quad: bound", quad_gen);

    ret += bound_test_prim("GenericPrimitive: bound", rand_prim_gen);

    ret += test_property(
        num_tests,
        0.05,
        "GenericPrimitive: hider",
        [&]() noexcept {
            const Vec3             o = posgen();
            const Vec3             d = make_normal(o - posgen());
            const Ray              ray{o, d};
            const constexpr size_t num_prims = 10;
            const auto radiuses              = rad_dist.template operator()<num_prims>(g);
            array<Vec3, num_prims>                  positions;
            positions[0] = o + d * radiuses[0];
            for(size_t i = 1; i < num_prims; ++i)
                positions[i] = positions[i - 1] + d * (radiuses[i - 1] + radiuses[i]);

            array<decltype(rand_prim_gen_l(radiuses[0], -d)), num_prims> prims;
            for(size_t i = 0; i < num_prims; ++i)
                prims[i] =
                    apply_transform(translate(positions[i]), rand_prim_gen_l(radiuses[i], -d));

            return pair{ray, prims};
        },
        [](const auto& feed) noexcept {
            const auto& [ray, prims] = feed;
            return hider(ray, prims);
        },
        [](const auto& testing, const auto) noexcept {
            const auto& [pidx, isect] = testing;
            return !isect || pidx != 0ul;
        });

    if(ret)
        fmt::print("{} tests failed\n", ret);
    else
        fmt::print("All tests passed\n");

    return ret;
}
