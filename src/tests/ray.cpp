// -*- C++ -*-
// ray.cpp --
#include "test_util.hpp"
#include <traversal/ray.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;
	Ray_ ray(Point(0), Normal(0,0,1));
	auto [o, d] {ray};
	const auto& [ro, rd] = ray;
	TEST_AND_COUT((is_same_v<decltype(o), Point>));
	TEST_AND_COUT((is_same_v<decltype(d), Normal>));
	TEST_AND_COUT((is_same_v<decltype(o), const Point>));
	TEST_AND_COUT((is_same_v<decltype(d), const Normal>));
	TEST_AND_COUT((is_same_v<decltype(o), Point&>));
	TEST_AND_COUT((is_same_v<decltype(d), Normal&>));
	TEST_AND_COUT((is_same_v<decltype(o), const Point&>));
	TEST_AND_COUT((is_same_v<decltype(d), const Normal&>));

	TEST_AND_COUT((is_same_v<decltype(ro), Point>));
	TEST_AND_COUT((is_same_v<decltype(rd), Normal>));
	TEST_AND_COUT((is_same_v<decltype(ro), const Point>));
	TEST_AND_COUT((is_same_v<decltype(rd), const Normal>));
	TEST_AND_COUT((is_same_v<decltype(ro), Point&>));
	TEST_AND_COUT((is_same_v<decltype(rd), Normal&>));
	TEST_AND_COUT((is_same_v<decltype(ro), const Point&>));
	TEST_AND_COUT((is_same_v<decltype(rd), const Normal&>));
	return 0;
}
