// csamisc_superfluoustemporary.cpp                                   -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "framework/analyser.hpp"
#include "framework/register_check.hpp"
#include "framework/cast_ptr.hpp"
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("superfluous-temporary");

static void
check_entry(cool::csabase::Analyser& analyser, clang::CXXConstructExpr const* expr)
{
    if (expr && expr->getNumArgs() == 1)
    {
        cool::cast_ptr<clang::MaterializeTemporaryExpr const> materialize(expr->getArg(0));
        cool::cast_ptr<clang::ImplicitCastExpr const>         implicit(materialize? materialize->GetTemporaryExpr(): 0);
        if (implicit)
        {
#if 0
            llvm::errs() << "ctor type=" << expr->getType().getAsString() << " "
                         << "arg-type=" << expr->getArg(0)->getType().getAsString() << " "
                         << "sub-expr-type=" << implicit->getSubExpr()->getType().getAsString() << " "
                         << "\n";
            analyser.report(expr, check_name, "superfluous temporary");
#endif
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name, &check_entry);
