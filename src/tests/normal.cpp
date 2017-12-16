// -*- C++ -*-
// normal.cpp --
#include <array>
#include <iostream>
#include <type_traits>
#include <core/types.hpp>
#include "test_util.hpp"

using namespace std;
using namespace yapt;

using NormalO = Normal<std::array>;
using PointO = Point<std::array>;
using Vec3r = Vec3<std::array>;

NormalO
get_test()
{
    return NormalO(10);
}

int main()
{
    std::cout << std::boolalpha;
    // TEST_AND_COUT((NormalO{}));
    TEST_AND_COUT((NormalO(1)).length());
    TEST_AND_COUT((NormalO(-32, 0.5, -14)).length());
    NormalO t = NormalO(NormalO(15) - NormalO(-6));
    TEST_AND_COUT(t.length());
    t = Vec3r(15,-12.5, 1);
    TEST_AND_COUT(t.length());
    t += 1;
    TEST_AND_COUT(t.length());
    TEST_AND_COUT((std::is_same<decltype(NormalO(-t)), decltype(t)>::value));
    // const real & t0 = t[0];
    TEST_AND_COUT(sqrt(t.dot(t)));
    // TEST_AND_COUT(sqrt(o));
    TEST_AND_COUT(get_test());
    TEST_AND_COUT((std::is_same<decltype(NormalO(3, -2, 4) + NormalO(-1,0.5,1)), NormalO>::value));
    TEST_AND_COUT((NormalO(3, -2, 4) + NormalO(-1,0.5,1)).length());
    TEST_AND_COUT(-NormalO(1));
    return 0;
}
