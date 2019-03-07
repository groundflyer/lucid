// -*- C++ -*-
// triangle.cpp
#include <traversal/triangle.hpp>

using namespace yapt;

int main()
{
    const Triangle triangle(Point(-1,-1,0), Point(0,1,0), Point(1,-1,0));
    const Normal dir(0,0,1);
    const Ray tohit(Point(0,0,-1), dir);
    const Ray tomiss(Point(0.6,0,-1), dir);
    int ret = !intersect(tohit, triangle);
    ret += intersect(tomiss, triangle);
    return ret;
}
