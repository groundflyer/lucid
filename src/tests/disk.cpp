// -*- C++ -*-
// disk.cpp
#include "test_util.hpp"
#include <primitives/disk.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Disk disk(Point(0,0,0), Normal(0,1,0), 1);
    const Ray tohit(Point(0,1,0), Normal(0,-1,0));
    const Ray tomiss1(Point(0,1,0), Normal(0,1,0));
    const Ray tomiss2(Point(1.1,1,0), Normal(0,-1,0));
    TEST_AND_COUT(intersect(tohit, disk));
    TEST_AND_COUT(intersect(tomiss1, disk));
    TEST_AND_COUT(intersect(tomiss2, disk));
    return 0;
}
