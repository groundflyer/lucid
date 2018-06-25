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
    random_device rd;
    default_random_engine gen(rd());
    RandomDistribution r_rng(-1.34, 2.6);
    RandomDistribution i_rng(-5, 5);
    TEST_AND_COUT(rand<real>(gen));
    TEST_AND_COUT(i_rng(gen));
    TEST_AND_COUT(r_rng(gen));
	COUT_ARRAY((rand<double, 4>(gen)));
	COUT_ARRAY((r_rng.operator()<8>(gen)));
	COUT_ARRAY((i_rng.operator()<20>(gen)));
    return 0;
}
