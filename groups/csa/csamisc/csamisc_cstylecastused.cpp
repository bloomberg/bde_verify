// csamisc_cstylecastused.cpp                                         -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("c-cast");

static void
check(cool::csabase::Analyser& analyser, clang::CStyleCastExpr const* expr)
{
    analyser.report(expr, check_name, "C-style cast is used")
        << expr->getSourceRange();
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &::check);
