// -*- C++ -*-
// sphere.cpp
#include <traversal/sphere.hpp>

using namespace yapt;

int main()
{
    const Sphere sphere(Point(0), 1);
    const Normal raydir(0,0,1);
    Ray tohit(Point(-raydir*10), raydir);
    Ray tomiss(Point(-raydir*10) + Point(1.1,0,0), raydir);
    const auto hit_isect = intersect(tohit, sphere);
    int ret = !hit_isect;
    ret += intersect(tomiss, sphere);
    return ret;
}
