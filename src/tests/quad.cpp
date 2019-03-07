// -*- C++ -*-
// quad.cpp
#include <traversal/quad.hpp>

using namespace std;
using namespace yapt;

int main()
{
    const Quad quad(Point(-1, -1, 0),
                    Point(-1, 1, 0),
                    Point(1, 1, 0),
                    Point(1, -1, 0));
    const Normal dir(0, 0, 1);
    const Ray tohit(Point(0, 0, -1), dir);
    const Ray tomiss(Point(2, 0, -1), dir);
    int ret = !intersect(tohit, quad);
    ret += intersect(tomiss, quad);
    return ret;
}
