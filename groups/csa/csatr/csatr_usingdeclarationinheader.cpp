// csamisc_usingdeclarationinheader.cpp                               -*-C++-*-
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

static std::string const check_name("using-declaration-in-header");

// ----------------------------------------------------------------------------

static void
using_declaration_in_header(cool::csabase::Analyser&  analyser,
                            clang::UsingDecl const   *decl)
{
    clang::DeclContext const* context(decl->getLexicalDeclContext());
    if (context->isFileContext()
        && analyser.get_location(decl).file() != analyser.toplevel()
        && !analyser.is_global_package())
    {
        analyser.report(decl, check_name, "TR16",
                        "Namespace level using declaration "
                        "in header file")
            << decl->getSourceRange();
    }
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name,
                                          &using_declaration_in_header);
