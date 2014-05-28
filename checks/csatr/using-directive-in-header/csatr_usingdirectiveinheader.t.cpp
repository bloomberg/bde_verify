// csatr_usingdirectiveinheader.t.cpp                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include "csatr_usingdirectiveinheader.t.hpp"
#include <bdes_ident.h>

using namespace bde_verify;
namespace bde_verify
{
    namespace csatr
    {
        static void f()
        {
            using namespace bde_verify;
        }
    }
}
