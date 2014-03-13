// csamisc_boolcomparison.t.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("boolcomparison");

// ----------------------------------------------------------------------------

static bool
is_bool_comparison(clang::Expr* expr0, clang::Expr* expr1)
{
    expr0 = expr0->IgnoreParenCasts();
    if (llvm::dyn_cast<clang::CXXBoolLiteralExpr>(expr0))
    {
        expr1 = expr1->IgnoreParenCasts();
        return expr0->getType().getUnqualifiedType().getCanonicalType()
            == expr1->getType().getUnqualifiedType().getCanonicalType();
    }
    return false;
}

// ----------------------------------------------------------------------------

static bool
is_comparison(clang::BinaryOperatorKind opcode)
{
    return opcode == clang::BO_LT
        || opcode == clang::BO_GT
        || opcode == clang::BO_LE
        || opcode == clang::BO_GE
        || opcode == clang::BO_EQ
        || opcode == clang::BO_NE
        ;
}

// ----------------------------------------------------------------------------

static void
check(bde_verify::csabase::Analyser& analyser, clang::BinaryOperator const* expr)
{
    if (is_comparison(expr->getOpcode())
        && (is_bool_comparison(expr->getLHS(), expr->getRHS())
            || is_bool_comparison(expr->getRHS(), expr->getLHS())))
    {
        analyser.report(expr, check_name, "BC01",
                        "Comparing a Boolean expression to a Boolean literal")
            << expr->getSourceRange();
    }
}

// ----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck register_check(check_name, &check);
