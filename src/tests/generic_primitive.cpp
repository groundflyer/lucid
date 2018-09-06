// -*- C++ -*-
// variant_primitive.cpp
#include "test_util.hpp"
#include <traversal/generic.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Normal raydir(0,0,1);
    Ray tohit(Point(-raydir*10), raydir);
    Ray tomiss(Point(-raydir*10) + Point(1.1,0,0), raydir);

    GenericPrimitive prim = Sphere(Point(0), 1);
    TEST_AND_COUT(sizeof(prim));
    TEST_AND_COUT(intersect(tohit, prim));
    TEST_AND_COUT(intersect(tomiss, prim));

    prim = AABB(Point(-1), Point(1));
    TEST_AND_COUT(intersect(tohit, prim));
    TEST_AND_COUT(intersect(tomiss, prim));

    prim = Disk(Point(0), -raydir, 1);
    TEST_AND_COUT(intersect(tohit, prim));
    TEST_AND_COUT(intersect(tomiss, prim));

    prim = Triangle(Point(-1,-1,0), Point(0,1,0), Point(1,-1,0));
    TEST_AND_COUT(intersect(tohit, prim));
    TEST_AND_COUT(intersect(tomiss, prim));

    prim = Plane(Point(0), -raydir);
    TEST_AND_COUT(intersect(tohit, prim));
    TEST_AND_COUT(intersect(Ray(Point(0), Normal(0,1,0)), prim));

    TEST_AND_COUT(compute_normal(tohit, intersect(tohit, prim), prim));
    return 0;
}
