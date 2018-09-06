// -*- C++ -*-
// sphere.cpp
#include "test_util.hpp"
#include <traversal/sphere.hpp>

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
    const auto hit_isect = intersect(tohit, sphere);
    int ret = 0;
    if(!hit_isect)
        ret++;
    if(intersect(tomiss, sphere))
        ret++;
    TEST_AND_COUT(compute_normal(tohit, hit_isect, sphere));
    return ret;
}
