
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
    auto mgen      = [&]() noexcept { return Mat4(dist.template operator()<16>(g)); };
    auto argen     = [&]() noexcept { return dist.template operator()<3>(g); };
    auto vgen      = [&]() noexcept { return Vec3(argen()); };
    auto pgen      = [&]() noexcept { return Point(argen()); };
    auto ngen      = [&]() noexcept { return Normal(argen()); };
    auto applytest = [](const Mat4& t, const auto& v) noexcept { return apply_transform(t, v); };

    microbench(
        log,
        n,
        "apply_transform Vec3",
        [&]() noexcept {
            return pair{mgen(), vgen()};
        },
        applytest);

    microbench(
        log,
        n,
        "apply_transform Point",
        [&]() noexcept {
            return pair{mgen(), pgen()};
        },
        applytest);

    microbench(
        log,
        n,
        "apply_transform Normal",
        [&]() noexcept {
            return pair{mgen(), ngen()};
        },
        applytest);

    microbench(log, n, "translate", pgen, [](const Point& p) noexcept { return translate(p); });

    microbench(log, n, "scale", vgen, [](const Vec3& v) noexcept { return scale(v); });

    microbench(
        log,
        n,
        "rotate",
        [&]() noexcept {
            return pair{dist(g), ngen()};
        },
        [](const auto a, const Normal& n) noexcept { return rotate(a, n); });

    microbench(
        log,
        n,
        "look_at",
        [&]() noexcept {
            return tuple{pgen(), pgen(), ngen()};
        },
        [](const Point& e, const Point& t, const Normal& u) noexcept { return look_at(e, t, u); });

    microbench(
        log, n, "basis_matrix", ngen, [](const Normal& n) noexcept { return basis_matrix(n); });

    return 0;
}
