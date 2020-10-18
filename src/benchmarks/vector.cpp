// -*- C++ -*-
// vector.cpp

#include "benchmark.hpp"

#include <base/vector.hpp>
#include <utils/tuple.hpp>
#include <utils/typeinfo.hpp>

#include <random>

using namespace std;
using namespace lucid;

template <typename T, std::size_t N, typename L, typename G>
static void
run_test(L& log, G& g, const std::size_t n) noexcept
{
    using Vec = Vector<T, N, array>;

    const auto typestring = get_typeinfo_string(Vec{});

    uniform_real_distribution<T> dist(-100000.f, 100000.f);
    auto                         vv   = [&]() noexcept { return Vec(generate<3>(dist, g)); };
    auto                         vgen = [&]() noexcept { return pair{vv(), vv()}; };

    microbench(
        log, n, "{} dot"_format(typestring), vgen, static_cast<T (*)(const Vec&, const Vec&)>(dot));
    microbench(log,
               n,
               "{} cross"_format(typestring),
               vgen,
               static_cast<Vec (*)(const Vec&, const Vec&)>(cross));
}

int
main(int argc, char* argv[])
{
    INIT_LOG

    random_device         rd;
    default_random_engine g(rd());

    run_test<float, 3>(log, g, n);
    run_test<float, 4>(log, g, n);
    run_test<double, 3>(log, g, n);
    run_test<double, 4>(log, g, n);

    return 0;
}
