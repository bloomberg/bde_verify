// csamisc_constantreturn.t.hpp                                       -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSAMISC_CONSTANTRETURN)
#define INCLUDED_CSAMISC_CONSTANTRETURN 1
#ident "$Id$"

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csamisc
    {
        struct constantreturn
        {
            static int x();
            static int f();
            static int g();
            static int h();
            static void v();
        };
    }
}

// -----------------------------------------------------------------------------

#endif
