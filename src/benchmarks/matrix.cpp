// -*- C++ -*-
// matrix.cpp
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
    auto mgen = [&]() noexcept { return Mat4(dist.template operator()<16>(g)); };
    microbench(
        log,
        n,
        "Mat4 dot",
        [&]() noexcept {
            return pair{mgen(), mgen()};
        },
        static_cast<Mat4 (*)(const Mat4&, const Mat4&)>(dot));
    microbench(
        log,
        n,
        "Mat4 dot Vec4",
        [&]() noexcept {
            return pair{mgen(), Vec4(dist.template operator()<4>(g))};
        },
        static_cast<Vec4 (*)(const Mat4&, const Vec4&)>(dot));

    microbench(log, n, "Mat4 transpose", mgen, static_cast<Mat4 (*)(const Mat4&)>(transpose));
    microbench(log, n, "Mat4 det", mgen, static_cast<real (*)(const Mat4&)>(det));
    microbench(log, n, "Mat4 inverse", mgen, static_cast<Mat4 (*)(const Mat4&)>(inverse));

    return 0;
}
