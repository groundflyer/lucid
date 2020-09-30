
#include "benchmark.hpp"

#include <base/rng.hpp>
#include <base/transforms.hpp>

using namespace std;
using namespace lucid;

int
main(int argc, char* argv[])
{
    std::FILE*   log = argc >= 2 ? std::fopen(argv[1], "a") : nullptr;
    const size_t n   = argc == 3 ? atoi(argv[2]) : 10000000;

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<real> dist(-100000_r, 100000_r);
    auto mgen      = [&]() noexcept { return Mat4(dist.template operator()<16>(g)); };
    auto argen     = [&]() noexcept { return dist.template operator()<3>(g); };
    auto vgen      = [&]() noexcept { return Vec3(argen()); };
    auto pgen      = [&]() noexcept { return Point(argen()); };
    auto ngen      = [&]() noexcept { return Normal(argen()); };
    auto applytest = [](auto& val) noexcept {
        auto& [t, v1, v2] = val;
        v2                = apply_transform(t, v1);
    };

    bench(
        log,
        n,
        "apply_transform Vec3",
        [&]() noexcept {
            return tuple{mgen(), vgen(), Vec3{0_r}};
        },
        applytest);

    bench(
        log,
        n,
        "apply_transform Point",
        [&]() noexcept {
            return tuple{mgen(), pgen(), Vec3{0_r}};
        },
        applytest);

    bench(
        log,
        n,
        "apply_transform Normal",
        [&]() noexcept {
            return tuple{mgen(), ngen(), Vec3{0_r}};
        },
        applytest);

    auto tsgen = [&]() noexcept { return tuple{pgen(), Mat4{}}; };

    bench(log, n, "translate", tsgen, [](auto& v) noexcept {
        auto& [p, m] = v;
        m            = translate(p);
    });

    bench(log, n, "scale", tsgen, [](auto& v) noexcept {
        auto& [p, m] = v;
        m            = scale(p);
    });

    bench(
        log,
        n,
        "rotate",
        [&]() noexcept {
            return tuple{dist(g), ngen(), Mat4{}};
        },
        [](auto& v) noexcept {
            auto& [a, n, m] = v;
            m               = rotate(a, n);
        });

    bench(
        log,
        n,
        "look_at",
        [&]() noexcept {
            return tuple{pgen(), pgen(), ngen(), Mat4{}};
        },
        [](auto& v) noexcept {
            auto& [e, t, u, m] = v;
            m                  = look_at(e, t, u);
        });

    bench(
        log,
        n,
        "basis_matrix",
        [&]() noexcept {
            return pair{ngen(), Mat3{}};
        },
        [](auto& v) noexcept {
            auto& [n, m] = v;
            m            = basis_matrix(n);
        });

    if(log) std::fclose(log);

    return 0;
}
