// -*- C++ -*-
// benchmark.hpp
//

#pragma once

#include <utils/timer.hpp>

#include <range/v3/view/generate_n.hpp>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <vector>

namespace lucid
{
template <typename G, typename R>
void
bench(std::size_t n, std::string_view name, G&& g, R&& r) noexcept
{
    auto vals = ranges::views::generate_n(g, n) | ranges::to<std::vector>();

    lucid::ElapsedTimer<> timer;

    for(auto& val: vals) r(val);

    const auto elapsed = timer.elapsed();

    fmt::print("{}: {} for {} runs; avg: {}\n",
               name,
               std::chrono::duration_cast<std::chrono::milliseconds>(elapsed),
               n,
               elapsed / n);
}
} // namespace lucid
