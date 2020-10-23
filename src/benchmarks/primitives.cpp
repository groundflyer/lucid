// -*- C++ -*-
// primitives.cpp
#include "benchmark.hpp"
#include <primitives/aabb.hpp>
#include <primitives/disk.hpp>
#include <primitives/generic.hpp>
#include <primitives/quad.hpp>
#include <primitives/sphere.hpp>
#include <primitives/triangle.hpp>
#include <ray_traversal/ray_traversal.hpp>
#include <utils/tuple.hpp>

#include <fmt/format.h>

#include <random>

using namespace std;
using namespace lucid;
using namespace fmt::literals;

static Vec3
vgen(uniform_real_distribution<real>& d, default_random_engine& g) noexcept
{
    return Vec3(generate<3>(d, g));
}

template <typename Prim, typename PG>
static void
bench_prim(LogFile&                         log,
           PG&&                             pg,
           uniform_real_distribution<real>& d,
           default_random_engine&           g,
           const size_t                     n,
           const std::string_view           name) noexcept
{
    auto randr = static_cast<real (*)(default_random_engine&)>(generate_canonical<real, 8>);

    microbench(
        log,
        n,
        "{} intersect"_format(name),
        [&]() noexcept {
            return pair{Ray(vgen(d, g), vgen(d, g)), pg()};
        },
        intersect);

    microbench(
        log,
        n,
        "{} normal"_format(name),
        [&]() noexcept {
            return pair{vgen(d, g), pg()};
        },
        static_cast<Vec3 (*)(const Vec3&, const Prim&)>(normal));

    microbench(
        log,
        n,
        "{} sample"_format(name),
        [&]() noexcept {
            return pair{Vec2(randr(g), randr(g)), pg()};
        },
        static_cast<Vec3 (*)(const Vec2&, const Prim&)>(sample));

    microbench(
        log, n, "{} centroid"_format(name), pg, static_cast<Vec3 (*)(const Prim&)>(centroid));

    microbench(log, n, "{} bound"_format(name), pg, static_cast<AABB (*)(const Prim&)>(bound));

    microbench(
        log,
        n,
        "{} apply_transform"_format(name),
        [&]() noexcept {
            return pair{Mat4(generate<16>(d, g)), pg()};
        },
        static_cast<Prim (*)(const Mat4&, const Prim&)>(apply_transform));
}

int
main(int argc, char* argv[])
{
    INIT_LOG

    random_device                   rd;
    default_random_engine           g(rd());
    uniform_real_distribution<real> d(-1000000_r, 1000000_r);

    bench_prim<AABB>(
        log, [&]() noexcept { return AABB(vgen(d, g), vgen(d, g)); }, d, g, n, "AABB");
    bench_prim<Disk>(
        log, [&]() noexcept { return Disk(vgen(d, g), vgen(d, g), d(g)); }, d, g, n, "Disk");
    bench_prim<Sphere>(
        log, [&]() noexcept { return Sphere(vgen(d, g), d(g)); }, d, g, n, "Sphere");
    bench_prim<Triangle>(
        log, [&]() noexcept { return generate<3>(vgen, d, g); }, d, g, n, "Triangle");
    bench_prim<Quad>(
        log, [&]() noexcept { return generate<4>(vgen, d, g); }, d, g, n, "Quad");

    return 0;
}
