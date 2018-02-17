#include <core/vec.hpp>
#include <core/mat.hpp>
#include <core/arrayview.hpp>

#include "test_util.hpp"

using namespace std;
using namespace yapt;


using Vec2f = Vector<float, 2, array>;
using Vec3f = Vector<float, 3, array>;
using Vec4f = Vector<float, 4, array>;

using Vec2i = Vector<int, 2, array>;
using Vec3i = Vector<int, 3, array>;
using Vec4i = Vector<int, 4, array>;

using Vec2fe = Vector<float, 2, ArrayView>;
using Vec3fe = Vector<float, 3, ArrayView>;
using Vec4fe = Vector<float, 4, ArrayView>;

using BigVec = Vector<double, 32, array>;

using ExArr3f = ArrayView<float, 3>;
using ExArr4f = ArrayView<float, 4>;

int main()
{
    cout << boolalpha;
    TEST_AND_COUT((is_array_v<Vec4f>));
    TEST_AND_COUT((is_scalar_v<Vec4f>));
    TEST_AND_COUT((is_array_v<ExArr4f>));
    TEST_AND_COUT((is_array_v<array<float, 4>>));
    TEST_AND_COUT((BigVec(0) + BigVec(10)));
    TEST_AND_COUT(-BigVec(10));
    TEST_AND_COUT((BigVec(10)/1));
    TEST_AND_COUT(reduce(BigVec(1)));
    TEST_AND_COUT((BigVec(2).dot(BigVec(4))));
    TEST_AND_COUT((BigVec(-2).cross(BigVec(4))));
    TEST_AND_COUT(Vec2f(2));
    TEST_AND_COUT(Vec3f(3));
    TEST_AND_COUT(Vec4f(4));
    TEST_AND_COUT((Vec4f(1) < 0.f));
    TEST_AND_COUT(Vec4i(Vec2i(1,2)));
    TEST_AND_COUT(Vec4i(Vec4f(1.5,2.5,3.5)));
    TEST_AND_COUT(Vec2i(Vec4f(1.5,2.5,3.5)));
    float a[4] = {1,2,3,4};
    COUT_ARRAY(a);
    TEST_AND_COUT(Vec3fe(ExArr3f(a)));
    TEST_AND_COUT(Vec3f(Vec3fe(ExArr3f(a))));
    TEST_AND_COUT(Vec4f(Vec3fe(ExArr3f(a))));
    TEST_AND_COUT(Vec2f(Vec3fe(ExArr3f(a))));
    TEST_AND_COUT(Vec3i(Vec3fe(ExArr3f(a))));
    TEST_AND_COUT(Vec2i(Vec3fe(ExArr3f(a))));
    TEST_AND_COUT(Vec4i(Vec3fe(ExArr3f(a))));
    TEST_AND_COUT((Vec4f(1) + Vec4f(2)));
    TEST_AND_COUT((Vec4f(1) + Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(9) - Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4i(1) + 2));
    TEST_AND_COUT(reduce(Vec4f(1)));
    COUT_ARRAY(a);
    TEST_AND_COUT((Vec4f(1) == Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(1) != Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(0) == Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(0) != Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(1) < Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(1) <= Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(0) > Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(0) >= Vec4fe(ExArr4f(a))));
    TEST_AND_COUT((Vec4f(0).dot(Vec4fe(ExArr4f(a)))));
    TEST_AND_COUT((Vec4f(0).cross(Vec4fe(ExArr4f(a)))));
    TEST_AND_COUT(length(Vec4f(10)));
    TEST_AND_COUT(distance(Vec4f(1), Vec4fe(ExArr4f(a))));
    TEST_AND_COUT(sum(Vec4fe(ExArr4f(a))));
    TEST_AND_COUT(avg(Vec4fe(ExArr4f(a))));
    TEST_AND_COUT(max(Vec4fe(ExArr4f(a))));
    TEST_AND_COUT(min(Vec4fe(ExArr4f(a))));
    Vec4f test(4,3,2,1);
	const auto& [x, y, z, w] = test;
	auto& [cx, cy, cz, cw] = test;
	cx = 5;
	TEST_AND_COUT((Vec4f(cx, cy, cz, cw)));
    test = Vec4f(100);
    Vec4fe teste = Vec4fe(ExArr4f(a));
    teste = test;
    TEST_AND_COUT(teste);
    teste /= 2.f;
    TEST_AND_COUT(teste);
	TEST_AND_COUT((Vec4f(x, y, z, w)));
    TEST_AND_COUT(normalize(Vec4f(1,2,3,4)));
    TEST_AND_COUT((Vector<float, 6, array>(Matrix<float, 2, 3, array>::unit().data())));
    TEST_AND_COUT((Matrix<float, 2, 3, array>(Vector<float, 6, array>(Matrix<float, 2, 3, array>::unit().data()).data())));

    return 0;
}
