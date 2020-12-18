// -*- C++ -*-
// benchmark.hpp
//

#pragma once

#include <utils/argparse.hpp>
#include <utils/timer.hpp>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/zip.hpp>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

#ifdef REPO_HASH
const constexpr char repo_hash[] = REPO_HASH;
#else
const constexpr char repo_hash[] = "nonrepo";
#endif

#define INIT_LOG                                                                               \
    constexpr std::tuple opts{                                                                 \
        lucid::argparse::option<'l'>(identity, "/dev/null", "log", "File to log to", "FILE"),  \
        lucid::argparse::option<'n'>(                                                          \
            [](std::string_view word) noexcept { return std::atoi(word.data()); },             \
            10000000,                                                                          \
            "runs",                                                                            \
            "Number of runs",                                                                  \
            "N")};                                                                             \
    lucid::argparse::ArgsRange args(argc, argv);                                               \
    const auto                 pr =                                                            \
        lucid::argparse::parse(opts, args, lucid::argparse::StandardErrorHandler(args, opts)); \
    LogFile      log(pr.get_opt<'l'>());                                                       \
    const size_t n = pr.get_opt<'n'>();

namespace lucid
{
struct LogFile
{
    std::FILE* file = nullptr;

    LogFile(const LogFile&) = delete;
    LogFile&
    operator=(const LogFile&) = delete;

    LogFile(){};
    LogFile(const std::string_view path) : file(std::fopen(path.data(), "a")) {}
    LogFile(LogFile&& other)
    {
        file       = other.file;
        other.file = nullptr;
    }

    LogFile&
    operator=(LogFile&& other)
    {
        file       = other.file;
        other.file = nullptr;
        return *this;
    }

    ~LogFile()
    {
        if(file) std::fclose(file);
    }

    void
    open(const std::string_view path)
    {
        if(file) std::fclose(file);

        file = std::fopen(path.data(), "a");
    }

    template <typename... Args>
    void
    append(Args&&... args) const
    {
        if(file) fmt::print(file, std::forward<Args>(args)...);
    }
};

namespace detail
{
template <typename T>
struct helper
{
    static constexpr bool is_tuple = false;

    template <typename R>
    using Results = std::vector<std::invoke_result_t<R, T>>;

    using Args = std::vector<T>;

    Args        args;
    std::size_t n;

    template <typename G>
    helper(G&& g, std::size_t _n) noexcept :
        args(ranges::views::generate_n(g, _n) | ranges::to<std::vector>()), n(_n)
    {
    }

    template <typename R>
    void
    run(R&& f, Results<R>& results) const noexcept
    {
        for(std::size_t i = 0; i < n; ++i) results[i] = f(args[i]);
    }
};

template <typename T1, typename T2>
struct helper<std::pair<T1, T2>>
{
    static constexpr bool is_tuple = true;

    template <typename R>
    using Results = std::vector<std::invoke_result_t<R, T1, T2>>;

    using Args = std::pair<std::vector<T1>, std::vector<T2>>;

    Args        args;
    std::size_t n;

    template <typename G>
    helper(G&& g, std::size_t _n) noexcept : n(_n)
    {
        auto& [va, vb] = args;
        va.resize(n);
        vb.resize(n);

        for(std::size_t i = 0; i < n; ++i)
        {
            const auto [a, b] = g();
            va[i]             = a;
            vb[i]             = b;
        }
    }

    template <typename R>
    void
    run(R&& f, Results<R>& results) const noexcept
    {
        const auto& [va, vb] = args;

        for(std::size_t i = 0; i < n; ++i) results[i] = f(va[i], vb[i]);
    }
};

template <typename... Ts>
struct helper<std::tuple<Ts...>>
{
    static constexpr bool is_tuple = true;

    template <typename R>
    using Results = std::vector<std::invoke_result_t<R, Ts...>>;

    using Args = std::tuple<std::vector<Ts>...>;

    Args        args;
    std::size_t n;

    template <typename G>
    helper(G&& g, std::size_t _n) noexcept : n(_n)
    {
        std::apply([&](auto&... vs) { (..., vs.resize(n)); }, args);

        for(std::size_t i = 0; i < n; ++i)
            std::apply([&](auto&... vs) { std::tie(vs[i]...) = g(); }, args);
    }

    template <typename R>
    void
    run(R&& f, Results<R>& results) const noexcept
    {
        for(std::size_t i = 0; i < n; ++i)
            std::apply([&](const auto&... vs) { results[i] = f(vs[i]...); }, args);
    }
};
} // namespace detail

template <typename G, typename R>
void
microbench(LogFile& log, const std::size_t n, std::string_view name, G&& g, R&& r) noexcept
{
    using Helper = detail::helper<std::invoke_result_t<G>>;

    using namespace std::literals::chrono_literals;

    Helper                               helper(g, n);
    typename Helper::template Results<R> results(n);

    lucid::ElapsedTimer<> timer;

    helper.run(r, results);

    const auto ns = timer.elapsed();

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    const auto t  = ns / n;
    fmt::print("{}: {} for {} runs; ~{}/call\n", name, ms, n, t);

    log.append(
        "{},{:%Y%m%d},{},{},{}\n", repo_hash, fmt::localtime(std::time(nullptr)), name, ns, n);
}
} // namespace lucid
