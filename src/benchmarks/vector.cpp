// -*- C++ -*-
// vector.cpp

#include <base/rng.hpp>
#include <base/vector.hpp>
#include <utils/timer.hpp>

#include <range/v3/view/generate_n.hpp>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <algorithm>
#include <cstdlib>
#include <tuple>
#include <vector>

using namespace std;
using namespace lucid;
using namespace ranges;

using Vec3f = Vector<float, 3>;

int
main(int argc, char* argv[])
{
    const size_t num_runs = argc == 2 ? atoi(argv[1]) : 10000000;

    random_device         rd;
    default_random_engine g(rd());

    RandomDistribution<float> dist(-100000.f, 100000.f);
    auto                      gen = [&]() { return Vec3f(dist.template operator()<3>(g)); };

    auto val_pairs = views::generate_n(
                         [&]() {
                             return tuple{gen(), gen(), Vec3f(0.f)};
                         },
                         num_runs) |
                     to<vector>();

    ElapsedTimer<> timer;

    for(auto& vp: val_pairs)
    {
        auto& [v1, v2, v3] = vp;
        v3                 = dot(v1, v2);
    }

    const auto elapsed = timer.elapsed();

    fmt::print("dot: {} for {} ~{}\n", elapsed, num_runs, elapsed / num_runs);

    return 0;
}
