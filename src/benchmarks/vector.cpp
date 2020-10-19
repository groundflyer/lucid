// -*- C++ -*-
// vector.cpp

#include "benchmark.hpp"

#include <base/vector.hpp>
#include <utils/tuple.hpp>
#include <utils/typeinfo.hpp>

#include <random>

using namespace std;
using namespace lucid;

template <typename T, std::size_t N, typename G>
static void
bench(LogFile& log, G& g, const size_t n) noexcept
{
    using Vec = Vector<T, N, array>;

    const auto typestring = get_typeinfo_string(Vec{});

    uniform_real_distribution<T> dist(T{-100000}, T{100000});
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

    bench<float, 3>(log, g, n);
    bench<float, 4>(log, g, n);
    bench<double, 3>(log, g, n);
    bench<double, 4>(log, g, n);

    return 0;
}
