// -*- C++ -*-
// vector.cpp

#include "benchmark.hpp"

#include <base/rng.hpp>
#include <base/types.hpp>

#include <tuple>

using namespace std;
using namespace lucid;

int
main(int argc, char* argv[])
{
    const size_t n = argc == 2 ? atoi(argv[1]) : 10000000;

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<real> dist(-100000_r, 100000_r);
    auto                     gen  = [&]() noexcept { return Vec3(dist.template operator()<3>(g)); };
    auto                     vgen = [&]() noexcept { return tuple{gen(), gen(), Vec3(0_r)}; };

    bench(n, "Vec3 dot", vgen, [](auto& v) noexcept {
        auto& [v1, v2, v3] = v;
        v3                 = dot(v1, v2);
    });

    bench(n, "Vec3 cross", vgen, [](auto& v) noexcept {
        auto& [v1, v2, v3] = v;
        v3                 = cross(v1, v2);
    });

    return 0;
}
