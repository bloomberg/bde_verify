// csabase_diagnosticfilter.t.hpp                                     -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_GROUPS_CFILTER_T)
#define INCLUDED_CSABASE_DIAGNOSTICFILTER_T 1
#ident "$Id$"
#include "csabase_diagnosticfilterextra.hpp"

// ----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        int* diagnosticFilterInlineDummy(int);
        int* diagnosticFilterSourceDummy(int);
    }
}

// ----------------------------------------------------------------------------

inline int*
cool::csabase::diagnosticFilterInlineDummy(int value)
{
    return &value;
}

// ----------------------------------------------------------------------------

#endif
