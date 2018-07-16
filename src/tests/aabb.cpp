// -*- C++ -*-
// sphere.cpp
#include "test_util.hpp"
#include <primitives/aabb.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Point b0(-1,-1,1);
    const Point b1(1,1,2);
    const AABB aabb(b0, b1);
    const Normal dir(0,0,1);
    const Ray tohit(Point(0), dir);
    const Ray tomiss(Point(1.1,0,0), dir);
    int ret = 0;
    TEST_AND_COUT(sizeof(aabb));
    const auto hit_isect = intersect(tohit, aabb);
    if(!hit_isect)
    {
        cout << "Hit failed\n";
        ret++;
    }
    else
        cout << "Hit OK\n";

    if(intersect(tomiss, aabb, Range<real>()))
    {
        cout << "Miss failed\n";
        ret++;
    }
    else
        cout << "Miss OK\n";

    return ret;
}
