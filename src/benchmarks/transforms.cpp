
#include "benchmark.hpp"

#include <base/rng.hpp>
#include <base/transforms.hpp>

using namespace std;
using namespace lucid;

int
main(int argc, char* argv[])
{
    INIT_LOG

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<real> dist(-100000_r, 100000_r);
    auto mgen  = [&]() noexcept { return Mat4(dist.template operator()<16>(g)); };
    auto argen = [&]() noexcept { return dist.template operator()<3>(g); };
    auto vgen  = [&]() noexcept { return Vec3(argen()); };
    auto pgen  = [&]() noexcept { return Vec3(argen()); };
    auto ngen  = [&]() noexcept { return make_normal(argen()); };

    microbench(
        log,
        n,
        "apply_transform",
        [&]() noexcept {
            return pair{mgen(), vgen()};
        },
        static_cast<Vec3 (*)(const Mat4&, const Vec3&)>(apply_transform));

    microbench(
        log,
        n,
        "apply_transform_p",
        [&]() noexcept {
            return pair{mgen(), pgen()};
        },
        static_cast<Vec3 (*)(const Mat4&, const Vec3&)>(apply_transform_p));

    microbench(
        log,
        n,
        "apply_transform_n",
        [&]() noexcept {
            return pair{mgen(), ngen()};
        },
        static_cast<Vec3 (*)(const Mat4&, const Vec3&)>(apply_transform_n));

    microbench(log, n, "translate", pgen, static_cast<Mat4 (*)(const Vec3&)>(translate));

    microbench(log, n, "scale", vgen, static_cast<Mat4 (*)(const Vec3&)>(scale));

    microbench(
        log,
        n,
        "rotate",
        [&]() noexcept {
            return pair{dist(g), ngen()};
        },
        static_cast<Mat4 (*)(const real, const Vec3&)>(rotate));

    microbench(
        log,
        n,
        "look_at",
        [&]() noexcept {
            return tuple{pgen(), pgen(), ngen()};
        },
        static_cast<Mat4 (*)(const Vec3&, const Vec3&, const Vec3&)>(look_at));

    microbench(log, n, "basis_matrix", ngen, static_cast<Mat3 (*)(const Vec3&)>(basis_matrix));

    return 0;
}
