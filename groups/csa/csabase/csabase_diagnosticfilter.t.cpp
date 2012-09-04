// csabase_diagnosticfilter.t.cpp                                     -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include "csabase_diagnosticfilter.t.hpp"

// ----------------------------------------------------------------------------

cool::csabase::DiagnosticFilterType*
cool::csabase::operator-(cool::csabase::DiagnosticFilterType value)
{
    return &value;
}
