// -*- C++ -*-
// transforms.cpp -- 
#include "test_util.hpp"
#include <core/transforms.hpp>

using namespace yapt;
using namespace std;

int main()
{
    int ret = 0;
    // translate
    {
        const Point p(0);
        const Vec3 delta(1);
        const auto m = translate(delta);
        const auto pres = apply_transform(m, p);
        if(all(pres != delta))
        {
            ++ret;
            cout << "Translate Point:\tFAIL\n";
        }
        else
            cout << "Translate Point:\tOK\n";

        const Normal n(0,1,0);
        const auto tn = apply_transform(m, n);
        if(all(tn != n))
        {
            ++ret;
            cout << "Translate Normal:\tFAIL\n";
        }
        else
            cout << "Translate Normal:\tOK\n";
    }

    // scale
    {
        const Point p(1);
        const Vec3 delta(2);
        const auto m = scale(delta);
        const auto tp = apply_transform(m, p);
        if(all(tp != delta))
        {
            ++ret;
            cout << "Scale Point:\tFAIL\n";
        }
        else
            cout << "Scale Point:\tOK\n";

        const Normal n(0,1,0);
        const auto tn = apply_transform(m, n);
        if(all(tn != n))
        {
            ++ret;
            cout << "Scale Normal:\tFAIL\n";
        }
        else
            cout << "Scale Normal:\tOK\n";

        const Vec3 v(1);
        const auto tv = apply_transform(m, v);
        if(all(tv != delta))
        {
            ++ret;
            cout << "Scale Vec3:\tFAIL\n";
        }
        else
            cout << "Scale Vec3:\tOK\n";
    }

    // rotate
    {
        const Point p(1,0,0);
        const Vec3 result(0,0,1);
        const auto m = rotate(math::PI<real>*0.5, Normal(0,1,0));
        const auto tp = apply_transform(m, p);
        if(all(tp != result))
        {
            ++ret;
            cout << "Rotate Point:\tFAIL\n";
        }
        else
            cout << "Rotate Point:\tOK\n";

        const Normal n(0,1,0);
        const auto tn = apply_transform(m, n);
        if(all(tn != result))
        {
            ++ret;
            cout << "Rotate Normal:\tFAIL\n";
        }
        else
            cout << "Rotate Normal:\tOK\n";

        const Vec3 v(1,0,0);
        const auto tv = apply_transform(m, v);
        if(all(tv != result))
        {
            ++ret;
            cout << "Rotate Vec3:\tFAIL\n";
        }
        else
            cout << "Rotate Vec3:\tOK\n";
    }

    // look at
    {
        const Point eye(10,10,10);
        const Point target(0);
        const Normal dir(target - eye);
        const auto m = look_at(eye, target);

        const auto tp = apply_transform(m, Point(0));
        if(all(!almost_equal(tp, eye)))
        {
            ++ret;
            cout << "Look At Point:\tFAIL " << tp << endl;
        }
        else
            cout << "Look At Point:\tOK\n";

        const Normal n(0,0,1);
        const auto tn = apply_transform(m, n);
        if(all(!almost_equal(tn, dir)))
        {
            ++ret;
            cout << "Look At Normal:\tFAIL " << tn << endl;
        }
        else
            cout << "Look At Normal:\tOK\n";
    }
    return ret;
}
