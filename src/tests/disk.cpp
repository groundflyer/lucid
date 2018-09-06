// -*- C++ -*-
// disk.cpp
#include "test_util.hpp"
#include <traversal/disk.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Disk disk(Point(0,0,0), Normal(0,1,0), 1);
    const Ray tohit(Point(0,1,0), Normal(0,-1,0));
    const Ray tomiss1(Point(0,1,0), Normal(0,1,0));
    const Ray tomiss2(Point(1.1,1,0), Normal(0,-1,0));
    const auto hit_isect = intersect(tohit, disk);
    int ret = 0;
    if(!hit_isect)
        ret++;
    if(intersect(tomiss1, disk))
        ret++;
    if(intersect(tomiss2, disk))
        ret++;
    TEST_AND_COUT(compute_normal(tohit, intersect(tohit, disk), disk));
    return ret;
}
