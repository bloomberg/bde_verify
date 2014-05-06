// csatr_globalfunctiononlyinsource.cpp                               -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("global-function-only-in-source");

// ----------------------------------------------------------------------------

namespace
{
    struct decl_not_in_toplevel
    {
        decl_not_in_toplevel(csabase::Analyser* analyser)
            : analyser_(analyser)
        {
        }
        bool operator()(clang::Decl const* decl) const
        {
            return analyser_->get_location(decl).file()
                != analyser_->toplevel();
        }
        csabase::Analyser* analyser_;
    };
}

// ----------------------------------------------------------------------------

static void
global_function_only_in_source(csabase::Analyser&   analyser,
                               clang::FunctionDecl const *decl)
{
    if (decl->isGlobal()
        && llvm::dyn_cast<clang::CXXMethodDecl>(decl) == 0
        && !analyser.is_component_header(analyser.toplevel())
        && analyser.get_location(decl).file() == analyser.toplevel()
        && std::find_if(decl->redecls_begin(), decl->redecls_end(),
                        decl_not_in_toplevel(&analyser))
            == decl->redecls_end()
        && !analyser.is_test_driver()
        && !decl->isMain())
    {
        analyser.report(decl, check_name, "TR10",
                        "Globally visible function '%0' "
                        "is not declared in header.")
                        << decl->getQualifiedNameAsString()
                        << decl->getNameInfo().getSourceRange();
    }
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck check(check_name,
                                          &global_function_only_in_source);
