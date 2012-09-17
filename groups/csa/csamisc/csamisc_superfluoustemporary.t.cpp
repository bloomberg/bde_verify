// csamisc_superfluoustemporary.t.cpp                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_superfluoustemporary.t.hpp"
#include <bdes_ident.h>

namespace cool
{
    namespace csamisc
    {
        namespace
        {
            struct bar
            {
            };

            struct foo
            {
                foo();
                foo(foo const&);
                foo(bar const&);                                    // IMPLICIT
            };
            foo::foo() {}
            foo::foo(foo const&) {}
            foo::foo(bar const&) {}
        }
    }
}

int main(int ac, char* av[])
{
    cool::csamisc::foo f0 = cool::csamisc::foo();
    cool::csamisc::foo b0 = cool::csamisc::bar();
    //foo f1;
}
