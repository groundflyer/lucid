// -*- C++ -*-
// plane.cpp
#include <traversal/plane.hpp>

using namespace yapt;

int main()
{
    const Plane plane(Point(0,0,0), Normal(0,1,0));
    const Ray tohit(Point(0,1,0), Normal(0,-1,0));
    const Ray tomiss(Point(0,1,0), Normal(0,1,0));
    int ret = 0;
    ret += !intersect(tohit, plane);
    ret += intersect(tomiss, plane);
    return ret;
}
