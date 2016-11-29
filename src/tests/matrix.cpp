#include <core/basic_types.hpp>
#include "test_util.hpp"

using namespace std;
using namespace yapt;


int main()
{
    array<real,4*4> data {};
    fill_range(data);
    Mat4 m2(data);
    Mat4 unit;

    TEST_AND_COUT((Mat4{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}));
    TEST_AND_COUT((Mat4(1) * -3.4));
    TEST_AND_COUT((Mat4(10) / 2));
    TEST_AND_COUT(Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1)));
    TEST_AND_COUT(sizeof(Mat4{}));
    TEST_AND_COUT(m2);
    TEST_AND_COUT(transpose(m2));
    TEST_AND_COUT(minor_matrix(m2, 3, 3));
    TEST_AND_COUT((Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1)) * m2));
    TEST_AND_COUT((Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1)) * Vec4(1,0,-2,4)));
    TEST_AND_COUT(Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1)) + m2);
    TEST_AND_COUT(Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1)) - m2);
    TEST_AND_COUT(-Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1)));
    TEST_AND_COUT(det(Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1))));
    m2 = Mat4(Vec4(1,0,-2,4), Vec4(0,1,0,3), Vec4(0,0,1,0), Vec4(0,-5,0,1));
    cout << "m2 = m1\n";
    m2.transpose();
    cout << "m2.transpose()\n";
    TEST_AND_COUT(m2);
    TEST_AND_COUT(cofactor(m2));
    TEST_AND_COUT(det(m2));

    TEST_AND_COUT(Mat4(1,0,0,5,
		       0,1,0,0,
		       0,0,1,0,
		       0,0,0,1) * Vec4(0,0,0,1));

    bool invertible = bool(m2);
    cout << "Is m2 invertible? "
	 << (invertible ? "Yes" : "No") << endl;
    if (invertible)
	{
	    TEST_AND_COUT(inverse(m2));
	    TEST_AND_COUT(m2 * inverse(m2));
	}

    for (auto elem : Mat4{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16})
	cout << elem << " ";
    cout << endl;

    return 0;
}
