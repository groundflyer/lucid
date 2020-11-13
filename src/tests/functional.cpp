// -*- C++ -*-
// functional.cpp
#include <fmt/format.h>

#include <utils/functional.hpp>

using namespace std;
using namespace lucid;

int
main()
{
    auto to_s = [](bool ok) { return ok ? "OK" : "FAIL"; };
    int  ret  = 0;

    // fold
    {
        const int  r  = fold(std::plus<int>{}, 1, 2, 3, 4);
        const bool ok = r == 10;
        fmt::print("Fold: {}\n", to_s(ok));
        ret += !ok;
    }

    // composition with constant passing
    {
        const auto                a = [](const int& n) { return n + 10; };
        const auto                b = [](const int n) { return n * 2; };
        const auto                c = [](const int& n) { return n / 5; };
        const auto                f = compose(a, b, c);
        std::decay_t<decltype(f)> f1;
        f1            = f;
        const int  n  = 10;
        const int  rc = f1(n);
        const int  rs = a(b(c(n)));
        const bool ok = rc == rs;
        fmt::print("Constant passing: {}\n", to_s(ok));
        ret += !ok;
    }

    // composition with reference passing
    {
        auto a = [](int& n) -> int& {
            n += 10;
            return n;
        };
        auto b = [](int& n) -> int& {
            n *= 2;
            return n;
        };
        auto c = [](int& n) -> int& {
            n /= 5;
            return n;
        };
        auto       f      = compose(a, b, c);
        int        n1     = 10;
        int        n2     = n1;
        int&       r1     = f(n1);
        int&       r2     = a(b(c(n2)));
        const bool val_ok = r1 == r2;
        const bool ref_ok = &n1 == &r1;
        fmt::print("Reference passing: value {}, ref {}\n", to_s(val_ok), to_s(ref_ok));
        ret += !val_ok;
        ret += !ref_ok;
    }

    // flip
    {
        auto        f  = flip(std::minus<float>{});
        const float a  = 10.f;
        const float b  = 2.f;
        const float r  = f(a, b);
        const float t  = b - a;
        const bool  ok = r == t;
        fmt::print("flip: {}\n", to_s(ok));
        ret += !ok;
    }

    // constructor composition
    {
        struct MakeInt
        {
            int n = 0;

            explicit constexpr MakeInt(int _n) : n(_n) {}
        };

        struct MakeFloat
        {
            float f = 0.f;

            explicit constexpr MakeFloat(const MakeInt& n) : f(n.n) {}
        };

        struct MakeDouble
        {
            double d = 0.;

            explicit constexpr MakeDouble(const MakeFloat& f) : d(f.f) {}
        };

        constexpr auto cc = compose(maker<MakeDouble>, maker<MakeFloat>, maker<MakeInt>);
        auto           dd = cc(10);
        static_assert(std::is_same_v<decltype(dd), MakeDouble>);
        bool ok = dd.d == 10.0;
        fmt::print("maker compose: {}\n", to_s(ok));
        ret += !ok;
    }
    return ret;
}
