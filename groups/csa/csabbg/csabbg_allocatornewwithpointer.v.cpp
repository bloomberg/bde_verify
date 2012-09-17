// csabbg_allocatornewwithpointer.v.cpp                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csabbg_allocatornewwithpointer.v.hpp"
#include <bdes_ident.h>
#include "bslma_allocator.h"

// -----------------------------------------------------------------------------

int main()
{
    char array[16];
    new(array) int(1);

    new int;
    new(BloombergLP::get_allocator()) int;
    new(&BloombergLP::get_allocator()) int;
    int i(0);
    new(&i) int;
}
