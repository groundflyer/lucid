#include <core/basic_types.hpp>

#include "test_util.hpp"

using namespace std;
using namespace yapt;


int main()
{
    Vec3 a(1.633, -.15, 2.43), b(-.53, -1.6, 2.5);
    auto test = a + b;
    test *= b;
    auto c = ((test / 4.f) ^ a) % b;
    Vec3 e = a * b;
    Vec3 n = Vec3(0);
    Vec3 equ_op;
    equ_op = 1;

    TEST_AND_COUT(sizeof(c));
    TEST_AND_COUT(sizeof(test));
    TEST_AND_COUT(test.size());
    TEST_AND_COUT(equ_op);
    TEST_AND_COUT(a);
    TEST_AND_COUT(b);
    TEST_AND_COUT(-a);
    TEST_AND_COUT(bool(a));
    TEST_AND_COUT((a == b));
    TEST_AND_COUT((a ^ b));
    TEST_AND_COUT(length(a));
    TEST_AND_COUT((a * b));
    TEST_AND_COUT(n);
    TEST_AND_COUT(bool(n));
    TEST_AND_COUT(length(e));
    e.normalize();
    cout << "e normalization\n";
    TEST_AND_COUT(e);
    TEST_AND_COUT(avg(e));
    TEST_AND_COUT(min(e));
    TEST_AND_COUT(max(e));
    TEST_AND_COUT(Vec3(Vec2(1, 2)));
    TEST_AND_COUT(Vec3(Vec4(1, 2, 3, 4)));

    return 0;
}
