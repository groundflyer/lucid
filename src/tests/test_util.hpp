// -*- C++ -*-
// test_util.hpp --
//

#ifndef __TEST_UTIL__
#define __TEST_UTIL__

#include <iostream>

#define TEST_AND_COUT(x)					\
    std::cout << "LINE: " << __LINE__ << std::endl;		\
    std::cout << "Performing " << #x << std::endl;		\
    std::cout << "Result is:\n";				\
    std::cout << x << std::endl;				\
    std::cout << "______________________________________\n";

#define COUT_ARRAY(x)						\
    std::cout << "Printing array " << #x << std::endl;		\
    for (const auto & i : x) std::cout << i << '\t';		\
    std::cout << "\n______________________________________\n";


#endif // __TEST_UTIL__
