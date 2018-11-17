// -*- C++ -*-
// rng.cpp -- 
// #include "property_test.hpp"
#include <base/types.hpp>
#include <base/rng.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
    const float lowest = -1e+3f;
    const float maxt = 1e+3f;
    RandomDistribution float_dist(lowest, maxt);
    random_device rd;
    default_random_engine gen(rd());
    cout << "lowest: " << lowest << endl
         << "max: " << maxt << endl
         << "vals: ";

    for(unsigned i = 0; i < 100; ++i)
        cout << float_dist(gen) << ' ';
    cout << endl;
    // RandomDistribution r_rng(-1.34, 2.6);
    // RandomDistribution i_rng(-5, 5);
    // TEST_AND_COUT(rand<real>(gen));
    // TEST_AND_COUT(i_rng(gen));
    // TEST_AND_COUT(r_rng(gen));
	// COUT_ARRAY((rand<double, 4>(gen)));
	// COUT_ARRAY((r_rng.operator()<8>(gen)));
	// COUT_ARRAY((i_rng.operator()<20>(gen)));
    return 0;
}
