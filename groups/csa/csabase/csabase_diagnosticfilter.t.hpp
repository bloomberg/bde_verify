// csabase_diagnosticfilter.t.hpp                                     -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_DIAGNOSTICFILTER)
#define INCLUDED_CSABASE_DIAGNOSTICFILTER 1
#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif
#ifndef INCLUDED_CSABASE_DIAGNOSTICFILTEREXTRA
#  include "csabase_diagnosticfilterextra.hpp"
#endif

// ----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        struct DiagnosticFilterType {};
        DiagnosticFilterType* operator+(DiagnosticFilterType);
        DiagnosticFilterType* operator-(DiagnosticFilterType);
    }
}

// ----------------------------------------------------------------------------

inline cool::csabase::DiagnosticFilterType*
cool::csabase::operator+(DiagnosticFilterType value)
{
    return &value;
}

// ----------------------------------------------------------------------------

#endif
