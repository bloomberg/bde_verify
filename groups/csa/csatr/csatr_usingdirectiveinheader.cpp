// csatr_usingdirectiveinheader.cpp                                   -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("using-directive-in-header");

// ----------------------------------------------------------------------------

static void
using_directive_in_header(csabase::Analyser&         analyser,
                          clang::UsingDirectiveDecl const *decl)
{
    clang::DeclContext const* context(decl->getLexicalDeclContext());
    if (context->isFileContext()
        && analyser.get_location(decl).file() != analyser.toplevel()
        && !decl->getNominatedNamespace()->isAnonymousNamespace()
        && !analyser.is_global_package()
        )
    {
        clang::NamedDecl const* name(decl->getNominatedNamespaceAsWritten());
        analyser.report(decl, check_name, "TR16",
                        "Namespace level using directive for '"
                        + name->getQualifiedNameAsString()
                        + "' in header file");
    }
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck check(check_name,
                                          &using_directive_in_header);
