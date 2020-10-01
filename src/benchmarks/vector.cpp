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
    INIT_LOG

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<real> dist(-100000_r, 100000_r);
    auto                     gen  = [&]() noexcept { return Vec3(dist.template operator()<3>(g)); };
    auto                     vgen = [&]() noexcept { return pair{gen(), gen()}; };

    microbench(log, n, "Vec3 dot", vgen, [](const Vec3& v1, const Vec3& v2) noexcept {
        return dot(v1, v2);
    });

    microbench(log, n, "Vec3 cross", vgen, [](const Vec3& v1, const Vec3& v2) noexcept {
        return cross(v1, v2);
    });

    return 0;
}
