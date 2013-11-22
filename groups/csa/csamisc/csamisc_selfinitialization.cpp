// csamisc_selfinitialization.cpp                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_visitor.h>
#include <csabase_localvisitor.h>
#include <functional>
#include <utility>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("self-init");

// -----------------------------------------------------------------------------

namespace
{
    struct match_var_decl
    {
        typedef clang::DeclRefExpr const* argument_type;
        match_var_decl(cool::csabase::Analyser& analyser, clang::VarDecl const* decl):
            analyser_(analyser),
            decl_(decl)
        {
        }
        void operator()(clang::DeclRefExpr const* ref) const
        {
            if (ref->getDecl() == this->decl_)
            {
                this->analyser_.report(this->decl_, check_name, "variable %0 used for self-initialization")
                    << this->decl_->getName()
                    << ref->getSourceRange();
            }
        }
        cool::csabase::Analyser& analyser_;
        clang::VarDecl const* decl_;
    };
}

// -----------------------------------------------------------------------------

static void
checker(cool::csabase::Analyser& analyser, clang::VarDecl const* decl)
{
    cool::local_visitor visitor(match_var_decl(analyser, decl));
    visitor.visit(decl);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name, &checker);
