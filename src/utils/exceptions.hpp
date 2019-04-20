// -*- C++ -*-
// exceptions.hpp
//

#pragma once


namespace yapt
{
    template <typename T>
    struct incorrect_range
    {
        T lower;
        T upper;
    };
}
