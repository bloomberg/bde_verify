// csabase_registercheck.h                                            -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_REGISTERCHECK)
#define INCLUDED_CSABASE_REGISTERCHECK 1
#ident "$Id$"

#include <utils/function.hpp>
#include <string>

// -----------------------------------------------------------------------------

namespace csabase {
    class Analyser;
    class PPObserver;
    class Visitor;
    class RegisterCheck;
} // close package namespace

// -----------------------------------------------------------------------------

class csabase::RegisterCheck
{
  public:
    template <typename T>
    RegisterCheck(std::string const& name,
                  void (*check)(csabase::Analyser&, T const*));
    RegisterCheck(std::string const& name,
                  utils::function<void(csabase::Analyser&,
                                       csabase::Visitor&,
                                       csabase::PPObserver&)>);
};

// -----------------------------------------------------------------------------

#endif
