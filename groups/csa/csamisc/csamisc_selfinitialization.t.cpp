// csamisc_selfinitialization.t.cpp                                   -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_selfinitialization.t.hpp"
#ident "$Id$"

int main()
{
    int f1(f1);
    int f2(f2 + 1);
    int f4(f2);

    int f3((f3=0,f3 + 1));
    int* p(new int[sizeof(p)]);
}
