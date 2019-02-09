// -*- C++ -*-
// static_span.cpp
#include <base/static_span.hpp>
#include <base/vector.hpp>
#include <iostream>

using namespace yapt;

int main()
{
    int arr[] = {1, 2, 3, 4};
    const StaticSpan span{arr};
    decltype(auto) el = std::get<0>(span);
    std::cout << &el << ": " << arr << std::endl;
    const Vector vec{span};
    decltype(auto) vel = std::get<0>(vec);
    std::cout << &vel << ": " << arr << std::endl;
    return &el != arr || &vel != arr;
}
