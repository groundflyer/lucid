// -*- C++ -*-
// vector.cpp

#include "benchmark.hpp"

#include <base/rng.hpp>
#include <base/vector.hpp>

#include <tuple>

using namespace std;
using namespace lucid;

using Vec3 = Vector<float, 3, array>;

int
main(int argc, char* argv[])
{
    INIT_LOG

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<float> dist(-100000.f, 100000.f);
    auto                      gen = [&]() noexcept { return Vec3(dist.template operator()<3>(g)); };
    auto                      vgen = [&]() noexcept { return pair{gen(), gen()}; };

    microbench(log, n, "Vec3 dot", vgen, static_cast<float (*)(const Vec3&, const Vec3&)>(dot));
    microbench(log, n, "Vec3 cross", vgen, static_cast<Vec3 (*)(const Vec3&, const Vec3&)>(cross));

    return 0;
}
