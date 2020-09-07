// -*- C++ -*-
// repeat.cpp
#include <utils/typelist.hpp>
#include <utility>

using namespace lucid;

static_assert(std::is_same_v<typelist<int, int, int, int>, typename repeat_type<4, int>::type>);

static_assert(std::is_same_v<std::integer_sequence<int, -10, 15, 20, -900>,
              typename join<std::integer_sequence<int, -10, 15>, std::integer_sequence<int, 20, -900>>::type>);

static_assert(std::is_same_v<std::integer_sequence<int, 10, 10, 10, 10, 10>, typename repeat_integer<5, int, 10>::type>);

static_assert(std::is_same_v<typename typelist<bool, int, float, double>::template at<2>, float>);

static_assert(typelist<bool, int, float, double>::template index<float>() == 2);


int main()
{
    return 0;
}
