// csamisc_contiguousswitch.t.cpp                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_contiguousswitch.t.hpp"

// -----------------------------------------------------------------------------

int cool::csamisc::contiguousswitch::f(int arg)
{
    char const* number(0);

    switch (arg)
    {
    case 0:
    case 5:
        number = "five";
        break;
    case 4:
        number = "four";
        break;
    case 3:
        number = "three";
        break;
    case 2:
        number = "two";
        break;
    case 1:
        number = "one";
        break;
    }

    switch (arg)
    {
    case 0:
    case 100:
        number = "hundred";
        break;
    case 5:
        number = "five";
        break;
    case 4:
        number = "four";
        break;
    case 3:
        number = "three";
        break;
    case 2:
        number = "two";
        break;
    case 1:
        number = "one";
        break;
    }

    return 0;
}
