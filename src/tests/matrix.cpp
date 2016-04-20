#include <core/basic_types.hpp>
#include "test_util.hpp"

using namespace std;
using namespace yapt;


int main()
{
    Vec4 a(1,0,-2,4), b(0,1,0,3), c(0,0,1,0), d(0,-5,0,1);
    Mat4 m1(a, b, c, d);
    array<real,4*4> data {};
    fill_range(data);
    Mat4 m2(data);
    Mat4 unit;
    Mat4 init_list_test = Mat4{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

    TEST_AND_COUT(init_list_test);
    TEST_AND_COUT((Mat4(1) * -3.4));
    TEST_AND_COUT((Mat4(10) / 2));
    TEST_AND_COUT(m1);
    TEST_AND_COUT(sizeof(m1));
    TEST_AND_COUT(m2);
    TEST_AND_COUT(transpose(m2));
    TEST_AND_COUT(minor_matrix(m2, 3, 3));
    TEST_AND_COUT((m1 * m2));
    TEST_AND_COUT((m1 * a));
    TEST_AND_COUT(m1 + m2);
    TEST_AND_COUT(m1 - m2);
    TEST_AND_COUT(-m1);
    TEST_AND_COUT(det(m1));
    m2 = m1;
    cout << "m2 = m1\n";
    m1.transpose();
    cout << "m1.transpose()\n";
    TEST_AND_COUT(m1);
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

    for (auto elem : Mat4(init_list_test))
	cout << elem << " ";
    cout << endl;

    return 0;
}
