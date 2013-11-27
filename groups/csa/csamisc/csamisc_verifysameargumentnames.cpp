// csamisc_verifysameargumentnames.cpp                                -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <sstream>
#ident "$Id: verify_same_argument_names.cpp 141 2011-09-29 18:59:08Z kuehl $"

// -----------------------------------------------------------------------------

static std::string const check_name("verify-same-argument-names");

// -----------------------------------------------------------------------------

namespace
{
    bool arg_names_match(clang::ParmVarDecl const* p0,
                         clang::ParmVarDecl const* p1)
    {
        std::string n0(p0->getNameAsString());
        std::string n1(p1->getNameAsString());
        bool rc = n0.empty() || n1.empty() || n0 == n1;
        return rc;
    }

    struct same_argument_names
    {
        same_argument_names(cool::csabase::Analyser* analyser,
                            clang::FunctionDecl const* current):
            analyser_(analyser),
            current_(current)
        {
        }

        void operator()(clang::Decl const* decl)
        {
            if (clang::FunctionDecl const* p =
                                   llvm::dyn_cast<clang::FunctionDecl>(decl)) {
                clang::FunctionDecl const* c(this->current_);
                unsigned n = p->getNumParams();
                if (n == c->getNumParams()) {
                    for (unsigned i = 0; i < n; ++i) {
                        const clang::ParmVarDecl *pp = p->getParamDecl(i);
                        const clang::ParmVarDecl *cp = c->getParamDecl(i);
                        if (!arg_names_match(pp, cp)) {
                            this->analyser_->report(c, check_name, "AN01: "
                                "parameter name mismatch for %ordinal0 "
                                "parameter; the other declaration used '%1'.")
                                << int(i + 1)
                                << pp->getName()
                                << cp->getSourceRange();
                        }
                    }
                }
            }
        }

        cool::csabase::Analyser*            analyser_;
        clang::FunctionDecl const* current_;
    };
}

// -----------------------------------------------------------------------------

static void
verify_arg_names_match(cool::csabase::Analyser& analyser,
                       clang::FunctionDecl const* decl)
{
    std::for_each(decl->redecls_begin(),
                  decl->redecls_end(),
                  same_argument_names(&analyser, decl));
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name, &verify_arg_names_match);
