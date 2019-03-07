// -*- C++ -*-
// variant_primitive.cpp
#include <traversal/generic.hpp>

using namespace yapt;

int main()
{
    const Normal raydir(0,0,1);
    Ray tohit(Point(-raydir*10), raydir);
    Ray tomiss(Point(-raydir*10) + Point(1.1,0,0), raydir);

    GenericPrimitive prim = Sphere(Point(0), 1);
    int ret = 0;
    ret += !intersect(tohit, prim);
    ret += intersect(tomiss, prim);

    prim = AABB(Point(-1), Point(1));
    ret += !intersect(tohit, prim);
    ret += intersect(tomiss, prim);

    prim = Disk(Point(0), -raydir, 1);
    ret += !intersect(tohit, prim);
    ret += intersect(tomiss, prim);

    prim = Triangle(Point(-1,-1,0), Point(0,1,0), Point(1,-1,0));
    ret += !intersect(tohit, prim);
    ret += intersect(tomiss, prim);

    prim = Plane(Point(0), -raydir);
    ret += !intersect(tohit, prim);
    ret += intersect(Ray(Point(0), Normal(0,1,0)), prim);

    return 0;
}
