// -*- C++ -*-
// rng.cpp -- 
#include "test_util.hpp"
#include <core/types.hpp>
#include <core/rng.hpp>

using namespace std;
using namespace yapt;

int main()
{
    TEST_AND_COUT(rand<real>());
    TEST_AND_COUT(rand<int>());
    TEST_AND_COUT((rand<int, -5, 2>()));
    TEST_AND_COUT((rand<double, -1, 1>()));
    return 0;
}
