// -*- C++ -*-
// sphere.cpp
#include "test_util.hpp"
#include <primitives/sphere.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Sphere sphere(Point(0,0,2), 1);
    const Normal dir(0,0,1);
    const Ray tohit(Point(0), dir);
    const Ray tomiss(Point(1.1,0,0), dir);
    TEST_AND_COUT(intersect(tohit, sphere, Range<real>()));
    TEST_AND_COUT(intersect(tomiss, sphere, Range<real>()));
    return 0;
}
