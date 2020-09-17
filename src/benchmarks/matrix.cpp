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
    const size_t n = argc == 2 ? atoi(argv[1]) : 10000000;

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<real> dist(-100000_r, 100000_r);
    auto mgen = [&]() noexcept { return Mat4(dist.template operator()<16>(g)); };
    bench(
        n,
        "Mat4 dot",
        [&]() noexcept {
            return tuple{mgen(), mgen(), Mat4(0_r)};
        },
        [](auto& v) noexcept {
            auto& [m1, m2, m3] = v;
            m3                 = dot(m1, m2);
        });
    bench(
        n,
        "Mat4 dot Vec4",
        [&]() noexcept {
            return tuple{mgen(), Vec4(dist.template operator()<4>(g)), Vec4(0_r)};
        },
        [](auto& v) noexcept {
            auto& [m, v1, v2] = v;
            v2                = dot(m, v1);
        });
    auto m2gen = [&]() noexcept { return pair{mgen(), Mat4(0_r)}; };
    bench(n, "Mat4 transpose", m2gen, [](auto& v) noexcept {
        auto& [m1, m2] = v;
        m2             = transpose(m1);
    });
    bench(
        n,
        "Mat4 det",
        [&]() noexcept {
            return pair{mgen(), 0_r};
        },
        [](auto& v) noexcept {
            auto& [m, d] = v;
            d            = det(m);
        });
    bench(n, "Mat4 inverse", m2gen, [](auto& v) noexcept {
        auto& [m1, m2] = v;
        m2             = inverse(m1);
    });
    return 0;
}
