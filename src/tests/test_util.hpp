// -*- C++ -*-
// test_util.hpp --
//

#ifndef __TEST_UTIL__
#define __TEST_UTIL__

#include <iostream>

#define TEST_AND_COUT(x)					\
    std::cout << "Performing " << #x << std::endl;		\
    std::cout << "Result is:\n";				\
    std::cout << x << std::endl;				\
    std::cout << "______________________________________\n";



#endif // __TEST_UTIL__
