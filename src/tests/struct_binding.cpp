// -*- C++ -*-
// struct_binding.cpp
#include <base/matrix.hpp>
#include <iostream>

using namespace std;
using namespace lucid;

using Mat4f = Matrix<float, 4, 4, array>;

int main()
{
    cout << boolalpha;
    const auto cm = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    const auto r0 = cm[0];
    cout << "is copy elem ok: " << (&r0[0] == &cm[0][0]) << endl;
    auto [cm0, cm1, cm2, cm3] = cm;
    cout << "copy bindings: "
         << cm0 << endl << cm1 << endl << cm2 << endl << cm3 << endl;
    cout << "is copy binding ok: " << (&cm[0][0] == &cm0[0]) << endl;
    cout << "difference is: " << (&cm[0][0] - &cm0[0]) << endl;
    auto& [rcm0, rcm1, rcm2, rcm3] = cm;
    cout << "is ref binding ok: " << (&cm[0][0] == &rcm0[0]) << endl;
    return 0;
}
