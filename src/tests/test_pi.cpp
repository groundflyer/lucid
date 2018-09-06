// -*- C++ -*-
// test_pi.cpp -- 
#include <iostream>
#include <iomanip>
#include <limits>

#include <base/math.hpp>


using namespace std;
using namespace yapt;


int main()
{
    cout << "float PI = " << setprecision(numeric_limits<float>::digits10+1)
         << math::PI<float> << endl;

    cout << "double PI = " << setprecision(numeric_limits<double>::digits10+1)
         << math::PI<double> << endl;

    return 0;
}
