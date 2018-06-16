// -*- C++ -*-
// debug.hpp --
//

#pragma once

#ifndef NDEBUG
	#include <cassert>
	#define ASSERT(CONDITION, MESSAGE) assert((CONDITION)&& MESSAGE)
#else
	#define ASSERT(CONDITION, MESSAGE)
#endif

#define CHECK_INDEX(idx, range) ASSERT(idx < range, "Index out of range.")
