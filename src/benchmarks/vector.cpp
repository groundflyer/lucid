// -*- C++ -*-
// vector.cpp

#include "benchmark.hpp"

#include <base/vector.hpp>

#include <tuple>

using namespace std;
using namespace lucid;

// template <typename T, std::size_t N>
// bm_vector(LogFile& log)

using Vec3f = Vector<float, 3, array>;
// using Vec3d = Vector<double, 3, array>;

int
main(int argc, char* argv[])
{
    INIT_LOG
    auto vgen = [&]() noexcept { return pair{Vec3f{}, Vec3f{}}; };

    microbench(log, n, "Vec3f dot", vgen, static_cast<float (*)(const Vec3f&, const Vec3f&)>(dot));
    microbench(
        log, n, "Vec3f cross", vgen, static_cast<Vec3f (*)(const Vec3f&, const Vec3f&)>(cross));

    return 0;
}
