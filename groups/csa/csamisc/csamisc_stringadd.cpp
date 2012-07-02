// -*-c++-*- groups/csa/csamisc/csamisc_stringadd.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#ident "$Id: c_style_cast_used.cpp 163 2012-02-24 23:44:49Z kuehl $"

// -----------------------------------------------------------------------------

static std::string const check_name("string-add");

static bool is_addition(cool::csabase::Analyser&  analyser,
                        clang::Expr const*        str,
                        clang::Expr const*        value,
                        clang::BinaryOperatorKind op)
{
    if (clang::StringLiteral const* lit = llvm::dyn_cast<clang::StringLiteral>(str->IgnoreParenCasts()))
    {
        llvm::APSInt length(32, false);
        llvm::APSInt zero(32, false);
        length = lit->getByteLength();
        zero = 0u;
        value = value->IgnoreParenCasts();
        llvm::APSInt result;
        return !value->isIntegerConstantExpr(result, *analyser.context())
            || (op == clang::BO_Add && (result < zero || length < result))
            || (op == clang::BO_Sub && (zero < result || length + result < zero))
            ;
        ;
    }
    return false;
}

static void
check(cool::csabase::Analyser& analyser, clang::BinaryOperator const* expr)
{
    if ((expr->getOpcode() == clang::BO_Add || expr->getOpcode() == clang::BO_Sub)
        && (is_addition(analyser, expr->getLHS(), expr->getRHS(), expr->getOpcode())
            || is_addition(analyser, expr->getRHS(), expr->getLHS(), expr->getOpcode()))
        )
    {
        analyser.report(expr->getOperatorLoc(), check_name, "%0 integer %1 string literal")
            << expr->getSourceRange()
            << (expr->getOpcode() == clang::BO_Add? "adding": "subtracting")
            << (expr->getOpcode() == clang::BO_Add? "to": "from")
            ;
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &::check);
