// csamisc_anonymousnamespaceinheader.t.cpp                           -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("anon-namespace");

static void anonymous_namespace_in_header(csabase::Analyser& analyser,
                                          clang::NamespaceDecl const* decl)
{
    if (decl->isAnonymousNamespace() && analyser.is_component_header(decl)) {
        analyser.report(decl, check_name, "ANS01",
                "Anonymous namespace in header", true);
    }
}

// -----------------------------------------------------------------------------

static csabase::RegisterCheck check(check_name, &anonymous_namespace_in_header);
