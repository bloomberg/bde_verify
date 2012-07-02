// csamisc_constantreturn.t.cpp                                       -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_constantreturn.t.hpp"

int cool::csamisc::constantreturn_f()
{
    return 0;
}

int cool::csamisc::constantreturn_g()
{
    return 5 + 8;
}

int cool::csamisc::constantreturn_h()
{
    {
        {
            return constantreturn_x();
        }
    }
}

void cool::csamisc::constantreturn_v()
{
    return;
}
