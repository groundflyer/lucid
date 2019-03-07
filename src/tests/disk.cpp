// -*- C++ -*-
// disk.cpp
#include <traversal/disk.hpp>

using namespace yapt;

int main()
{
    const Disk disk(Point(0,0,0), Normal(0,1,0), 1);
    const Ray tohit(Point(0,1,0), Normal(0,-1,0));
    const Ray tomiss1(Point(0,1,0), Normal(0,1,0));
    const Ray tomiss2(Point(1.1,1,0), Normal(0,-1,0));
    const auto hit_isect = intersect(tohit, disk);
    int ret = !hit_isect;
    ret += intersect(tomiss1, disk);
    ret += intersect(tomiss2, disk);
    return ret;
}
