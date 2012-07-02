// csabase_registercheck.h                                            -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_REGISTERCHECK)
#define INCLUDED_CSABASE_REGISTERCHECK 1
#ident "$Id$"

#include <cool/function.hpp>
#include <string>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class Analyser;
        class PPObserver;
        class Visitor;
        class RegisterCheck;
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::RegisterCheck
{
public:
    template <typename T>
    RegisterCheck(std::string const& name,
                  void (*check)(cool::csabase::Analyser&, T const*));
    RegisterCheck(std::string const& name,
                  cool::function<void(cool::csabase::Analyser&,
                                      cool::csabase::Visitor&,
                                      cool::csabase::PPObserver&)>);
};

// -----------------------------------------------------------------------------

#endif
