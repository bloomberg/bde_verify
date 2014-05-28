// csatr_usingdeclarationinheader.t.cpp                               -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include "csatr_usingdeclarationinheader.t.hpp"
#include <bdes_ident.h>

using bde_verify::csamisc::foo;
namespace bde_verify
{
    namespace csatr
    {
        using bde_verify::csamisc::foo;
        static void f()
        {
            using bde_verify::csamisc::foo;
        }
    }
}
