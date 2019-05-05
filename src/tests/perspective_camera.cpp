// -*- C++ -*-
// perspective_camera.cpp
#include "test_util.hpp"
#include <cameras/perspective.hpp>

using namespace std;
using namespace lucid;

int main()
{
    Point origin(-1, 0, -1);
    Point center(0);
    PerspectiveCamera cam(0.5_r, look_at(origin, center));
    const auto ray = cam(Vec2(0.5, 0.5));
    const auto& [o, d] = ray;
    TEST_AND_COUT(origin);
    TEST_AND_COUT(center);
    TEST_AND_COUT(o);
    TEST_AND_COUT(d);
    return 0;
}
