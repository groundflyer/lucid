// -*- C++ -*-
// rng.cpp -- 
#include "test_util.hpp"
#include <core/types.hpp>
#include <core/rng.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    RNG r_rng(-1.34, 2.6);
    RNG i_rng(-5, 5);
    TEST_AND_COUT(rand<real>());
    TEST_AND_COUT(i_rng());
    TEST_AND_COUT(r_rng());
	COUT_ARRAY((rand<double, 4>()));
	COUT_ARRAY((r_rng.operator()<8>()));
	COUT_ARRAY((i_rng.operator()<20>()));
    return 0;
}
