// -*- C++ -*-
// normal.cpp --
#include <type_traits>
#include <core/types.hpp>
#include "test_util.hpp"

using namespace std;
using namespace yapt;


int main()
{
    std::cout << std::boolalpha;
    TEST_AND_COUT((Normal(1)).length());
    TEST_AND_COUT((Normal(-32, 0.5, -14)).length());
    Normal t = Normal(Normal(15) - Normal(-6));
    TEST_AND_COUT(t.length());
    t = Vec3(15,-12.5, 1);
    TEST_AND_COUT(t.length());
    t += 1;
    TEST_AND_COUT(t.length());
    TEST_AND_COUT((std::is_same<decltype(Normal(-t)), decltype(t)>::value));
    // const real & t0 = t[0];
    TEST_AND_COUT(sqrt(t.dot(t)));
    // TEST_AND_COUT(sqrt(o));
    TEST_AND_COUT((std::is_same<decltype(Normal(3, -2, 4) + Normal(-1,0.5,1)), Normal>::value));
    TEST_AND_COUT((Normal(3, -2, 4) + Normal(-1,0.5,1)).length());
    TEST_AND_COUT(-Normal(1));
    TEST_AND_COUT(Point(Vec3(1,2,3)));
    TEST_AND_COUT(Normal(Vec3(1,2,3)));
    Point pt;
    pt = Vec3(1,2,3);
    TEST_AND_COUT(pt);
    TEST_AND_COUT((std::is_same<decltype(Point(2) + 1), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) + Point(1)), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) - 1), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) - Point(1)), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) * 1), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) * Point(1)), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) / 1), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(Point(2) / Point(1)), Point>::value));
    TEST_AND_COUT((std::is_same<decltype(pt += 1), Point&>::value));
    TEST_AND_COUT((std::is_same<decltype(pt -= 1), Point&>::value));
    TEST_AND_COUT((std::is_same<decltype(pt *= 1), Point&>::value));
    TEST_AND_COUT((std::is_same<decltype(pt /= 1), Point&>::value));
    TEST_AND_COUT((Point(2) + 1));
    TEST_AND_COUT((Point(2) + Point(1)));
    TEST_AND_COUT((Point(2) - 1));
    TEST_AND_COUT((Point(2) - Point(1)));
    TEST_AND_COUT((Point(2) * 1));
    TEST_AND_COUT((Point(2) * Point(1)));
    TEST_AND_COUT((Point(2) / 1));
    TEST_AND_COUT((Point(2) / Point(1)));
    TEST_AND_COUT((pt += 1));
    TEST_AND_COUT((pt -= 1));
    TEST_AND_COUT((pt *= 1));
    TEST_AND_COUT((pt /= 1));
    return 0;
}
