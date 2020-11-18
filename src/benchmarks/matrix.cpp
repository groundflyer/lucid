// -*- C++ -*-
// matrix.cpp
#include "benchmark.hpp"

#include <base/types.hpp>
#include <utils/tuple.hpp>

#include <random>

using namespace std;
using namespace lucid;

int
main(int argc, char* argv[])
{
    INIT_LOG

    random_device                        rd;
    default_random_engine                g(rd());
    std::uniform_real_distribution<real> dist(-100000_r, 100000_r);
    auto mgen = [&]() noexcept { return Mat4(generate<16>(dist, g)); };
    microbench(
        log,
        n,
        "Mat4 dot",
        [&]() noexcept {
            return pair{mgen(), mgen()};
        },
        dot);
    microbench(
        log,
        n,
        "Mat4 dot Vec4",
        [&]() noexcept {
            return pair{mgen(), Vec4(generate<4>(dist, g))};
        },
        dot);

    microbench(log, n, "Mat4 transpose", mgen, transpose);
    microbench(log, n, "Mat4 det", mgen, det);
    microbench(log, n, "Mat4 inverse", mgen, inverse);

    return 0;
}
