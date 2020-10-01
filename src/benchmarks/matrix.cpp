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
        [](const Mat4& m1, const Mat4& m2) noexcept { return dot(m1, m2); });
    microbench(
        log,
        n,
        "Mat4 dot Vec4",
        [&]() noexcept {
            return pair{mgen(), Vec4(dist.template operator()<4>(g))};
        },
        [](const Mat4& m, const Vec4& v) noexcept { return dot(m, v); });

    microbench(log, n, "Mat4 transpose", mgen, [](const Mat4& m) noexcept { return transpose(m); });
    microbench(log, n, "Mat4 det", mgen, [](const Mat4& m) noexcept { return det(m); });
    microbench(log, n, "Mat4 inverse", mgen, [](const Mat4& m) noexcept { return inverse(m); });

    return 0;
}
