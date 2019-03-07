// -*- C++ -*-
// sphere.cpp
#include <traversal/aabb.hpp>
#include <iostream>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Point b0(-1,-1,1);
    const Point b1(1,1,2);
    const AABB aabb(b0, b1);
    const Normal dir(0,0,1);
    const Ray tohit(Point(0), dir);
    const Ray tomiss(Point(1.1,0,0), dir);
    int ret = 0;
    const auto hit_isect = intersect(tohit, aabb);
    if(!hit_isect)
    {
        cout << "Hit failed\n";
        ret++;
    }
    else
        cout << "Hit OK\n";

    if(intersect(tomiss, aabb))
    {
        cout << "Miss failed\n";
        ret++;
    }
    else
        cout << "Miss OK\n";

    return ret;
}
