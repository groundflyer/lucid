// -*- C++ -*-
// perspective_camera.cpp
#include "test_util.hpp"
#include <cameras/perspective.hpp>

using namespace std;
using namespace yapt;

int main()
{
    PerspectiveCamera cam;
    const auto ray = cam(NDC(0.5, 0.5));
    const auto& [o, d] = ray;
    TEST_AND_COUT(o);
    TEST_AND_COUT(d);
    return 0;
}
