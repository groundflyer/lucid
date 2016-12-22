#include "test_util.hpp"
#include <core/basic_types.hpp>


using namespace std;
using namespace yapt;

using Vec3u = Vector<unsigned, 3>;
using Vec4u = Vector<unsigned, 4>;


int main()
{
    TEST_AND_COUT((Vec4{1,2,3,4,7,8,9}));
    TEST_AND_COUT(sizeof((Vec4{})));
    TEST_AND_COUT(Vec4(1., -3., 2., 4.));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) == Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) == Vec4(1., -3., 2., 4.)));
    TEST_AND_COUT((Vec4(1., 3., 2., 4.) > Vec4(-1., -3., -2., -4.)));
    TEST_AND_COUT((Vec4(1., 3., 2., 4.) > Vec4(5., -3., -2., -4.)));
    TEST_AND_COUT((Vec4(1., 3., 2., 4.) >= Vec4(-1., -3., -2., -4.)));
    TEST_AND_COUT((Vec4(1., 3., 2., 4.) >= Vec4(1., 3., 2., 4.)));
    TEST_AND_COUT((Vec4(1., 3., 2., 4.) <= Vec4(-1., -3., -2., -4.)));
    TEST_AND_COUT((Vec4(1., 3., 2., 4.) <= Vec4(1., 3., 2., 4.)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) * Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) + Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) - Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) / Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) ^ Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT((Vec4(1., -3., 2., 4.) % Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT(length(Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT(normalize(Vec4(-1., 3., -2., 4)));
    TEST_AND_COUT(sum(Vec4(1,1,1,1)));
    TEST_AND_COUT(avg(Vec4(1,2,3,4)));
    TEST_AND_COUT(max(Vec4(5,-2,4,1)));
    TEST_AND_COUT(min(Vec4(5,-2,4,1)));
    TEST_AND_COUT(Vec4(Vec3(1.,2.,3)));
    TEST_AND_COUT(Vec3((Vec4(1.,2.,3.,4))));
    TEST_AND_COUT(Vec4u(Vec4(1.5,2.8,3.6,4.1)));
    TEST_AND_COUT(Vec4(Vec3(1.,2.,3)));
    TEST_AND_COUT(Vec2i(10 ,15));

    return 0;
}
