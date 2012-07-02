// csabase_checkregistry.h                                            -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_CHECKREGISTRY)
#define INCLUDED_CSABASE_CHECKREGISTRY 1
#ident "$Id$"

#include <cool/function.hpp>
#include <string>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class Analyser;
        class Visitor;
        class PPObserver;
        class CheckRegistry;
    }
}

// -----------------------------------------------------------------------------
// This class maintains a list of all the registered checks. Essentially, it
// is a map of (name, function) pairs where the function does the necessary
// operations to subscribe to the suitable events on the visitor object its
// gets passed.

class cool::csabase::CheckRegistry
{
public:
    typedef cool::function<void(cool::csabase::Analyser&,
                                cool::csabase::Visitor&,
                                cool::csabase::PPObserver&)> Subscriber;
    static void add_check(std::string const&, Subscriber);
    static void attach(cool::csabase::Analyser&,
                       cool::csabase::Visitor&,
                       cool::csabase::PPObserver&);
};

// -----------------------------------------------------------------------------

#endif
