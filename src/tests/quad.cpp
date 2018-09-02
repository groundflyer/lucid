// -*- C++ -*-
// quad.cpp
#include "test_util.hpp"
#include <primitives/quad.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const Quad quad(Point(-1, -1, 0),
                    Point(-1, 1, 0),
                    Point(1, 1, 0),
                    Point(1, -1, 0));
    const Normal dir(0, 0, 1);
    const Ray tohit(Point(0, 0, -1), dir);
    const Ray tomiss(Point(2, 0, -1), dir);
    TEST_AND_COUT(intersect(tohit, quad));
    TEST_AND_COUT(intersect(tomiss, quad));
    TEST_AND_COUT(compute_normal(tohit, intersect(tohit, quad), quad));
    return 0;
}
