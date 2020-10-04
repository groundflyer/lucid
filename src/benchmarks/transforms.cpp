
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
    auto ngen  = [&]() noexcept { return normalize(Vec3(argen())); };

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

    microbench(log, n, "translate", pgen, [](const Vec3& p) noexcept { return translate(p); });

    microbench(log, n, "scale", vgen, [](const Vec3& v) noexcept { return scale(v); });

    microbench(
        log,
        n,
        "rotate",
        [&]() noexcept {
            return pair{dist(g), ngen()};
        },
        [](const auto a, const Vec3& n) noexcept { return rotate(a, n); });

    microbench(
        log,
        n,
        "look_at",
        [&]() noexcept {
            return tuple{pgen(), pgen(), ngen()};
        },
        [](const Vec3& e, const Vec3& t, const Vec3& u) noexcept { return look_at(e, t, u); });

    microbench(
        log, n, "basis_matrix", ngen, [](const Vec3& n) noexcept { return basis_matrix(n); });

    return 0;
}
