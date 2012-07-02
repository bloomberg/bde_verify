// csamisc_contiguousswitch.cpp                                       -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#ident "$Id: contiguous_switch.cpp 165 2012-03-06 00:42:25Z kuehl $"

// -----------------------------------------------------------------------------

static std::string const check_name("contiguous-switch");

static void
check(cool::csabase::Analyser& analyser, clang::SwitchStmt const* stmt)
{
    bool initial(true);
    llvm::APSInt value;
    for (clang::SwitchCase const* label(stmt->getSwitchCaseList()); label; label = label->getNextSwitchCase())
    {
        clang::CaseStmt const* case_(llvm::dyn_cast<clang::CaseStmt>(label));
        if (case_)
        {
            clang::Expr const* expr(case_->getLHS());
            llvm::APSInt result;
            if (expr->isIntegerConstantExpr(result, *analyser.context()))
            {
                if (!initial
                    && ++value != result
                    && (label->getNextSwitchCase() || result.toString(10) != "0"))
                {
                    analyser.report(label, check_name, "unexpected gap in case labels: expected %0, got %1")
                        << value.toString(10)
                        << result.toString(10)
                        << expr->getSourceRange();
                    value = result;
                }
                if (initial)
                {
                    value = result;
                    initial = false;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &::check);
