// -*- C++ -*-
// sphere.cpp
#include "test_util.hpp"
#include <primitives/sphere.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Sphere sphere(Point(0), 1);
    const Normal raydir(0,0,1);
    Ray tohit(Point(-raydir*10), raydir);
    Ray tomiss(Point(-raydir*10) + Point(1.1,0,0), raydir);
    TEST_AND_COUT(sizeof(sphere));
    TEST_AND_COUT(intersect(tohit, sphere));
    TEST_AND_COUT(intersect(tomiss, sphere));
    return 0;
}
