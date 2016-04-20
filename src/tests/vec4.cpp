#include "test_util.hpp"
#include <core/basic_types.hpp>


using namespace std;
using namespace yapt;

using Vec3u = Vector<unsigned, 3>;
using Vec4u = Vector<unsigned, 4>;


int main()
{
    Vec4 a(1., -3., 2., 4.), b(-1., 3., -2., 4);
    auto test = a + b;
    test *= b;
    auto c = (test / 4);
    test *= c;
    Vec4 init_list_test = Vec4{1,2,3,4,7,8,9};
    TEST_AND_COUT(init_list_test);
    TEST_AND_COUT(sizeof(test));
    TEST_AND_COUT(a);
    TEST_AND_COUT(b);
    TEST_AND_COUT((a == b));
    TEST_AND_COUT((a ^ b));
    TEST_AND_COUT((a % b));
    TEST_AND_COUT(Vec4((Vec3(1.,2.,3).cbegin())));
    TEST_AND_COUT(Vec3((Vec4(1.,2.,3.,4).cbegin())));
    TEST_AND_COUT(Vec4u(Vec4(1.5,2.8,3.6,4.1)));
    TEST_AND_COUT(Vec4(Vec3(1.,2.,3)));

    return 0;
}
