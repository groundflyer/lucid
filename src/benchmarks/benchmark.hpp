// -*- C++ -*-
// benchmark.hpp
//

#pragma once

#include <utils/timer.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/generate_n.hpp>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <vector>

#include <cstdio>

#ifdef REPO_HASH
const constexpr char repo_hash[] = REPO_HASH;
#else
const constexpr char repo_hash[] = "nonrepo";
#endif

namespace lucid
{
template <typename G, typename R>
void
bench(std::FILE* log, const std::size_t n, std::string_view name, G&& g, R&& r) noexcept
{
    auto vals = ranges::views::generate_n(g, n) | ranges::to<std::vector>();

    lucid::ElapsedTimer<> timer;

    for(auto& val: vals) r(val);

    const auto ns = timer.elapsed();

    const auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    const auto avg = ns / n;
    fmt::print("{}: {} for {} runs; avg: {}\n", name, ms, n, avg);

    if(log)
    {
        std::time_t t = std::time(nullptr);
        fmt::print(log, "{},{:%Y%m%d},{},{},{}\n", repo_hash, fmt::localtime(t), name, ns, n);
    }
}
} // namespace lucid
