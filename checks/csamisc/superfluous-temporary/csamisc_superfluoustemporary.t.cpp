// csamisc_superfluoustemporary.t.cpp                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_superfluoustemporary.t.hpp"
#include <bdes_ident.h>

namespace bde_verify
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
    bde_verify::csamisc::foo f0 = bde_verify::csamisc::foo();
    bde_verify::csamisc::foo b0 = bde_verify::csamisc::bar();
    //foo f1;
}
