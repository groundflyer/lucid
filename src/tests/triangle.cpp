// -*- C++ -*-
// triangle.cpp
#include "test_util.hpp"
#include <primitives/triangle.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Triangle triangle(Point(-1,-1,0), Point(0,1,0), Point(1,-1,0));
    const Normal dir(0,0,1);
    const Ray tohit(Point(0,0,-1), dir);
    const Ray tomiss(Point(0.6,0,-1), dir);
    TEST_AND_COUT(sizeof(triangle));
    TEST_AND_COUT(intersect(tohit, triangle));
    TEST_AND_COUT(intersect(tomiss, triangle));
    return 0;
}
