// -*- C++ -*-
// debug.hpp --
//

#pragma once


#ifndef NDEBUG
	#include <cassert>
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif
