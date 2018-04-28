// -*- C++ -*-
// plane.cpp
#include "test_util.hpp"
#include <primitives/plane.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Plane plane(Point(0,0,0), Normal(0,1,0));
    const Ray tohit(Point(0,1,0), Normal(0,-1,0));
    const Ray tomiss(Point(0,1,0), Normal(0,1,0));
    TEST_AND_COUT(intersect(tohit, plane));
    TEST_AND_COUT(intersect(tomiss, plane));
    return 0;
}
