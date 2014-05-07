// -*-c++-*- utils/array.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(UTILS_ARRAY_HPP)
#define UTILS_ARRAY_HPP 1
#ident "$Id: array.hpp 141 2011-09-29 18:59:08Z kuehl $"

#include <stddef.h>

// -----------------------------------------------------------------------------

namespace utils
{
template <typename T, size_t Size>
T* begin(T (&array)[Size])
{
    return array;
}
template <typename T, size_t Size>
T* end(T (&array)[Size])
{
    return array + Size;
}
}

// -----------------------------------------------------------------------------

#endif /* UTILS_ARRAY_HPP */
