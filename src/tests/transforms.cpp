// -*- C++ -*-
// transforms.cpp -- 
#include "test_util.hpp"
#include <core/math/transforms.hpp>
#include <math/pi.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

ostream &
operator<<(ostream & os, const glm::mat4 & m)
{
    auto m2 = glm::transpose(m);
    os << "[";
    for (size_t i = 0; i < 4; ++i)
	{
	    for (size_t j = 0; j < 4; ++j)
		{
		    os << m2[i][j];

		    if (j < 4 - 1)
			os << ",\t";
		}

	    if (i < 4 - 1)
		os << std::endl;
	}
    os << "]";


    return os;
}

int main()
{
    TEST_AND_COUT(yapt::translate(yapt::Vec3(1,2,3)));
    TEST_AND_COUT(yapt::scale(yapt::Vec3(1,2,3)));
    TEST_AND_COUT(yapt::rotate(2.4, yapt::Vec3(0,1,0)));
    TEST_AND_COUT(yapt::homogenize_point(yapt::Vec3(5,0,0)));
    TEST_AND_COUT(yapt::dehomogenize(yapt::Vec4(1,1,1,2)));

    TEST_AND_COUT(yapt::dehomogenize
		  (yapt::scale(yapt::Vec3(2)) * yapt::Vec4(1)));
    TEST_AND_COUT(yapt::translate(yapt::Vec3(5,0,0)) *
		  homogenize_point(yapt::Vec3(0)));

    TEST_AND_COUT(yapt::apply_transform_point
		  (yapt::rotate(yapt::PI<yapt::real>*0.5, yapt::normalize(yapt::Vec3(0,1,0))) *
		   yapt::translate(yapt::Vec3(10,0,0)),
		   yapt::Vec3(10,0,0)));

    TEST_AND_COUT(yapt::apply_transform_vector
		  (yapt::rotate(yapt::PI<yapt::real>*0.5, yapt::normalize(yapt::Vec3(0,1,0))) *
		   yapt::translate(yapt::Vec3(10,0,0)),
		   yapt::Vec3(10,0,0)));

    TEST_AND_COUT(yapt::apply_transform_normal
		  (yapt::rotate(yapt::PI<yapt::real>*0.5, yapt::normalize(yapt::Vec3(0,1,0))) *
		   yapt::translate(yapt::Vec3(10,0,0)),
		   yapt::Vec3(10,0,0)));

    glm::vec4 ver = glm::rotate(glm::mat4(1), yapt::PI<float>*0.5f, glm::normalize(glm::vec3(0,1,0)))
	* glm::vec4(20,0,0,1);
    cout << "glm rotate = "<<ver[0]<<", "<<ver[1]<<", "<<ver[2]<< endl;

    TEST_AND_COUT(yapt::look_at(yapt::Vec3(1., 1., 1.), yapt::Vec3(0), yapt::Vec3(0,1,0)));
    TEST_AND_COUT(glm::lookAt(glm::vec3(1,1,1), glm::vec3(0), glm::vec3(0,1,0)));

    return 0;
}
