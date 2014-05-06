// csabase_checkregistry.h                                            -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_CHECKREGISTRY)
#define INCLUDED_CSABASE_CHECKREGISTRY 1
#ident "$Id$"

#include <utils/function.hpp>
#include <string>

// -----------------------------------------------------------------------------

namespace csabase {
    class Analyser;
    class Visitor;
    class PPObserver;
    class CheckRegistry;
} // close package namespace

// -----------------------------------------------------------------------------
// This class maintains a list of all the registered checks. Essentially, it
// is a map of (name, function) pairs where the function does the necessary
// operations to subscribe to the suitable events on the visitor object its
// gets passed.

class csabase::CheckRegistry
{
public:
    typedef utils::function<void(csabase::Analyser&,
                                csabase::Visitor&,
                                csabase::PPObserver&)> Subscriber;
    static void add_check(std::string const&, Subscriber);
    static void attach(csabase::Analyser&,
                       csabase::Visitor&,
                       csabase::PPObserver&);
};

// -----------------------------------------------------------------------------

#endif
