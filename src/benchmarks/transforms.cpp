
#include "benchmark.hpp"

#include <base/transforms.hpp>
#include <utils/tuple.hpp>

#include <random>

using namespace std;
using namespace lucid;

int
main(int argc, char* argv[])
{
    INIT_LOG

    random_device                   rd;
    default_random_engine           g(rd());
    uniform_real_distribution<real> dist(-100000_r, 100000_r);
    auto                            mgen  = [&]() noexcept { return Mat4(generate<16>(dist, g)); };
    auto                            argen = [&]() noexcept { return generate<3>(dist, g); };
    auto                            vgen  = [&]() noexcept { return Vec3(argen()); };
    auto                            pgen  = [&]() noexcept { return Vec3(argen()); };
    auto                            ngen  = [&]() noexcept { return make_normal(argen()); };

    microbench(
        log,
        n,
        "apply_transform",
        [&]() noexcept {
            return pair{mgen(), vgen()};
        },
        apply_transform);

    microbench(
        log,
        n,
        "apply_transform_p",
        [&]() noexcept {
            return pair{mgen(), pgen()};
        },
        apply_transform_p);

    microbench(
        log,
        n,
        "apply_transform_n",
        [&]() noexcept {
            return pair{mgen(), ngen()};
        },
        apply_transform_n);

    microbench(log, n, "translate", pgen, translate);

    microbench(log, n, "scale", vgen, scale);

    microbench(
        log,
        n,
        "rotate",
        [&]() noexcept {
            return pair{dist(g), ngen()};
        },
        lucid::rotate);

    microbench(
        log,
        n,
        "look_at",
        [&]() noexcept {
            return tuple{pgen(), pgen(), ngen()};
        },
        look_at);

    microbench(log, n, "basis_matrix", ngen, basis_matrix);

    return 0;
}
