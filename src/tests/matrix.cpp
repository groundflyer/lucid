#include <core/mat.hpp>
#include "test_util.hpp"

using namespace std;
using namespace yapt;

using ExArr16f = ArrayView<float, 16>;

using Mat4f = Matrix<float, 4, 4, array>;
using Mat3f = Matrix<float, 3, 3, array>;
using Vec4f = Vector<float, 4, array>;

using Mat4fe = Matrix<float, 4, 4, ArrayView>;
using Vec4fe = Vector<float, 4, ArrayView>;


int main()
{
    std::cout << std::boolalpha;
    TEST_AND_COUT((is_array_v<Mat4f>));
    TEST_AND_COUT((is_scalar_v<Mat4f>));
    TEST_AND_COUT((Mat4f{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}));
    TEST_AND_COUT(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));
    auto m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    TEST_AND_COUT(m[0]);
    TEST_AND_COUT(sizeof(Matrix<unsigned char,4,4,array>(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)));
    TEST_AND_COUT(Mat4f::unit());
    auto u = m;

	const auto cu = m;
	TEST_AND_COUT(cu[0]);
	TEST_AND_COUT((is_same_v<decltype(cu[0]), const Vector<float, 4, ArrayView>>));

	auto& [cu0, cu1, cu2, cu3] = cu;
	TEST_AND_COUT((is_same_v<decltype(cu0), const Vector<float, 4, ArrayView>>));
	TEST_AND_COUT((cu0.cbegin() == cu.cbegin()));
	
	auto u00 = u[0];
	TEST_AND_COUT((is_same_v<decltype(u00), Vector<float, 4, ArrayView>>));
	auto& [u0, u1, u2, u3] = u;
	TEST_AND_COUT((u00.cbegin() == u0.cbegin()));
	TEST_AND_COUT((is_same_v<decltype(u00), decltype(u0)>));

	auto [v0, v1, v2, v3] = u;
	TEST_AND_COUT((u00.cbegin() == v0.cbegin()));
	TEST_AND_COUT((is_same_v<decltype(u00), decltype(v0)>));

    u[0][0] = 10;
	u0 += 100;
	u00 += 1000;
    u[0] += 2;
    u[1] = Vec4f(58,59,60,61);
    u[2][3] = 34;
    u[3] *= Vec4f(15, -5, 0.5, 31);
    TEST_AND_COUT(u);
    COUT_ARRAY(u.data());
	TEST_AND_COUT(Mat4f(cu0, cu1, cu2, cu3));
    TEST_AND_COUT(Mat4f(Vec4f(1,0,-2,4), Vec4f(0,1,0,3), Vec4f(0,0,1,0), Vec4f(0,-5,0,1)));
    TEST_AND_COUT(Mat3f(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)));
    TEST_AND_COUT(Mat4f(Mat3f(1,2,3,4,5,6,7,8,9)));
    float a[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    COUT_ARRAY(a);
    TEST_AND_COUT(Mat4fe(ExArr16f(a)));
    auto extMat = Mat4fe(ExArr16f(a));
    extMat = Mat4f(12);
    TEST_AND_COUT(extMat);
    u = extMat;
    TEST_AND_COUT(u);
    extMat = 1;
    extMat += Mat4f::unit();
    extMat *= 10;
    extMat /= 2;
    TEST_AND_COUT(extMat);
    u = Mat4f(10);
    TEST_AND_COUT(u);
    TEST_AND_COUT((Mat4f(10) + extMat));
    TEST_AND_COUT((Mat4f(10) + 1));
    TEST_AND_COUT((Mat4f(1) * -3.4));
    TEST_AND_COUT((Mat4f(10).dot(Vec4f(5))));
    TEST_AND_COUT((Mat4f::unit().dot(Mat4f(5))));
    TEST_AND_COUT((Mat4f(10) / 2));
    TEST_AND_COUT(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16).transpose());
    TEST_AND_COUT(minor_matrix((Mat4f::unit() + 1) * 2, 3, 3));
    TEST_AND_COUT(cofactor(Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)));
    TEST_AND_COUT(det(Mat4f::unit()));
    TEST_AND_COUT(det((Mat4f::unit() + 1) * 2));

    bool invertible = det((Mat4f::unit() + 1) * 2) != 0;
    cout << "Is m2 invertible? "
    	 << (invertible ? "Yes" : "No") << endl;
    if (invertible)
    {
    	TEST_AND_COUT(inverse((Mat4f::unit() + 1) * 2));
    	TEST_AND_COUT(transpose((Mat4f::unit() + 1) * 2));
    	TEST_AND_COUT((((Mat4f::unit() + 1) * 2).dot(inverse((Mat4f::unit() + 1) * 2))));
    	TEST_AND_COUT(det((((Mat4f::unit() + 1) * 2).dot(inverse((Mat4f::unit() + 1) * 2)))));
    }

    TEST_AND_COUT((Matrix<float, 16, 32, array>(0.16f).dot(Matrix<float, 32, 12, array>(-4.5f))));
    TEST_AND_COUT((Matrix<float, 2, 4, array>::unit() > Matrix<float, 2, 4, array>(0.5)));

    return 0;
}
