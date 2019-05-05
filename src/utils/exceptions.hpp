// -*- C++ -*-
// exceptions.hpp
//

#pragma once


namespace lucid
{
    template <typename T>
    struct incorrect_range
    {
        T lower;
        T upper;
    };
}
