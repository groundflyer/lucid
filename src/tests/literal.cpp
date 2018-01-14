// -*- C++ -*-
// literal.cpp --
#include "test_util.hpp"
#include <core/types.hpp>
#include <type_traits>

using namespace std;
using namespace yapt;

int main()
{
	cout << boolalpha;
	TEST_AND_COUT((-1.2_r < 0_r));
	TEST_AND_COUT((is_same_v<decltype(1.2_r), yapt::real>));
	TEST_AND_COUT((is_same_v<decltype(2_r), yapt::real>));
	TEST_AND_COUT((is_same_v<decltype(-0.4389_r), yapt::real>));
	TEST_AND_COUT((is_same_v<decltype(-293894_r), yapt::real>));
	return 0;
}
