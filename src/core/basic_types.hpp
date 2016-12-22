// -*- C++ -*-
// types.hpp --
//

#pragma once

#include <math/vec.hpp>
#include <math/matrix.hpp>


namespace yapt
{
#ifdef DOUBLE_PRECISION
    typedef double real;
#else
    typedef float real;
#endif

    using Vec2 = Vector<real, 2>;
    using Vec3 = Vector<real, 3>;
    using Vec4 = Vector<real, 4>;

    using Mat2 = Matrix<real, 2>;
    using Mat3 = Matrix<real, 3>;
    using Mat4 = Matrix<real, 4>;

    using RGB = Vector<float, 3>;
    using RGBA = Vector<float, 4>;

    using Vec3i = Vector<int, 3>;

    using Vec2i = Vector<int, 2>;
    using Vec2u = Vector<unsigned, 2>;
}
