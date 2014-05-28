// csabase_diagnosticfilterextra.hpp                                  -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_DIAGNOSTICFILTEREXTRA)
#define INCLUDED_CSABASE_DIAGNOSTICFILTEREXTRA 1
#ident "$Id$"

// -----------------------------------------------------------------------------

namespace csabase {
    int* diagnosticFilterExtraInlineDummy(int);
} // close package namespace

// ----------------------------------------------------------------------------

inline int*
csabase::diagnosticFilterExtraInlineDummy(int value)
{
    return &value;
}

// -----------------------------------------------------------------------------

#endif
