// -*- C++ -*-
// sphere.cpp
#include "test_util.hpp"
#include <primitives/aabb.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const AABB aabb(Point(-1,-1,1), Point(1,1,2));
    const Normal dir(0,0,1);
    const Ray tohit(Point(0), dir);
    const Ray tomiss(Point(1.1,0,0), dir);
    TEST_AND_COUT(sizeof(aabb));
    TEST_AND_COUT(intersect(tohit, aabb, Range<real>()));
    TEST_AND_COUT(intersect(tomiss, aabb, Range<real>()));
    return 0;
}
