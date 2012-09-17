// csamisc_constantreturn.t.cpp                                       -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_constantreturn.t.hpp"
#include <bdes_ident.h>

int cool::csamisc::constantreturn::f()
{
    return 0;
}

int cool::csamisc::constantreturn::g()
{
    return 5 + 8;
}

int cool::csamisc::constantreturn::h()
{
    {
        {
            return constantreturn::x();
        }
    }
}

void cool::csamisc::constantreturn::v()
{
    return;
}
