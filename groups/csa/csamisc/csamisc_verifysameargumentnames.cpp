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
    bool
    verify_same_argument_names_compare(clang::ParmVarDecl const* p0, clang::ParmVarDecl const* p1)
    {
        std::string n0(p0->getNameAsString());
        std::string n1(p1->getNameAsString());
        bool rc = n0.empty() || n1.empty() || n0 == n1;
        return rc;
    }

    struct same_argument_names
    {
        same_argument_names(cool::csabase::Analyser* analyser, clang::FunctionDecl const* current):
            analyser_(analyser),
            current_(current)
        {
        }
        void operator()(clang::Decl const* decl)
        {
            if (clang::FunctionDecl const* p = llvm::dyn_cast<clang::FunctionDecl>(decl))
            {
                clang::FunctionDecl const* c(this->current_);

                if (std::distance(p->param_begin(), p->param_end()) == std::distance(c->param_begin(), c->param_end()))
                {
                    typedef clang::FunctionDecl::param_const_iterator It;
                    for (std::pair<It, It> pair(p->param_begin(), c->param_begin());
                         p->param_end() != (pair = std::mismatch(pair.first, p->param_end(), pair.second,
                                                                 verify_same_argument_names_compare)).first; )
                    {
                        this->analyser_->report(c, check_name,
                                                "parameter name mismatch for %ordinal0 parameter; the other declaration used '%1'.")
                            << int(1 + pair.first - p->param_begin())
                            << (*pair.first)->getName()
                            << (*pair.second)->getSourceRange();
                        ++pair.first;
                        ++pair.second;
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
verify_same_argument_names(cool::csabase::Analyser& analyser, clang::FunctionDecl const* decl)
{
    std::for_each(decl->redecls_begin(), decl->redecls_end(), ::same_argument_names(&analyser, decl));
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name, &::verify_same_argument_names);
