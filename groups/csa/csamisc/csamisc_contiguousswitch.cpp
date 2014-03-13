// csamisc_contiguousswitch.cpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_cast_ptr.h>
#include <algorithm>
#include <sstream>
#ident "$Id: contiguous_switch.cpp 165 2012-03-06 00:42:25Z kuehl $"

// ----------------------------------------------------------------------------

static std::string const check_name("contiguous-switch");

// ----------------------------------------------------------------------------

static bool
getValue(bde_verify::csabase::Analyser& analyser,
         clang::Expr const*       expr,
         long&                    value)
{
    llvm::APSInt result;
    return expr->isIntegerConstantExpr(result, *analyser.context())
        && std::istringstream(result.toString(10)) >> std::skipws >> value;
}

// ----------------------------------------------------------------------------

static void
checkBreak(bde_verify::csabase::Analyser& analyser,
           clang::SwitchCase const* label,
           bool                     hasBreak) { // case zero has NO break!
    
}

// ----------------------------------------------------------------------------

static void
checkSwitch(bde_verify::csabase::Analyser& analyser, clang::SwitchStmt const* stmt)
{
    typedef std::vector<clang::SwitchCase const*>::const_iterator const_iterator;
    std::vector<clang::SwitchCase const*> cases;
    for (clang::SwitchCase const* label(stmt->getSwitchCaseList());
         label; label = label->getNextSwitchCase()) {
        cases.push_back(label);
    }
    if (cases.empty()) {
        analyser.report(stmt, check_name, "ES01", "Empty switch statement");
        return;
    }
    std::reverse(cases.begin(), cases.end());
    
    const_iterator it(cases.begin()), end(cases.end());
    if (bde_verify::csabase::cast_ptr<clang::DefaultStmt> label = *it) {
        analyser.report(*it, check_name, "SD01", "Switch starts with default")
            << label.get();
        checkBreak(analyser, *it, true);
        ++it;
    }
    else if (bde_verify::csabase::cast_ptr<clang::CaseStmt> label = *it) {
        long result(0);
        if (!getValue(analyser, label->getLHS(), result) || result != 0) {
            analyser.report(*it, check_name, "SZ01",
                            "Switch doesn't start with `case 0:`")
                << label.get();
            checkBreak(analyser, *it, true);
        }
        else {
            checkBreak(analyser, *it, false);
            ++it;
        }
    }
    else {
        analyser.report(*it, check_name, "US01", "Unknown switch case")
            << cases.front();
    }

    if (it != end) {
        long previous(0);
        if (bde_verify::csabase::cast_ptr<clang::CaseStmt> label = *it) {
            if (!getValue(analyser, label->getLHS(), previous)) {
                analyser.report(*it, check_name, "NC01",
                                "Can't get value from case label")
                    << *it;
            }
        }
        else if (bde_verify::csabase::cast_ptr<clang::DefaultStmt> label = *it) {
            if (it + 1 != end) {
                analyser.report(label.get(), check_name, "MD01",
                                "`default:` label in the middle of labels")
                    << *it;
            }
        }
        checkBreak(analyser, *it, true);
        ++it;

        bool default_at_end(false);
        for (; it != end; ++it) {
            long value(0);
            if (bde_verify::csabase::cast_ptr<clang::CaseStmt> label = *it) {
                if (!getValue(analyser, label->getLHS(), value)) {
                    analyser.report(*it, check_name, "NC01",
                                    "Can't get value from case label")
                        << *it;
                }
                else if ((1 < previous && previous - 1 != value)
                         || value == 0
                         || (previous <= value)) {
                    analyser.report(*it, check_name, "LO01",
                                    "Case label out of order: "
                                    "previous=%0 value=%1")
                        << previous << value;
                }
                previous = value;
            }
            else if (bde_verify::csabase::cast_ptr<clang::DefaultStmt> label = *it) {
                if (it + 1 != end) {
                    analyser.report(label.get(), check_name, "MD01",
                                    "`default:` label in the middle of labels")
                        << *it;
                }
                else {
                    default_at_end = true;
                }
            }
            checkBreak(analyser, *it, true);
        }
        if (!default_at_end) {
            analyser.report(stmt, check_name, "ED01",
                            "Switch doesn't end with `default:` label");
        }
    }
}

// ----------------------------------------------------------------------------

static void
check(bde_verify::csabase::Analyser& analyser, clang::FunctionDecl const* decl)
{
    if (decl->getNameAsString() != "main" || !decl->hasBody()) {
        return;
    }
    if (bde_verify::csabase::cast_ptr<clang::CompoundStmt> body = decl->getBody()) {
        typedef clang::CompoundStmt::const_body_iterator const_iterator;
        for (const_iterator it(body->body_begin()), end(body->body_end());
             it != end; ++it) {
            if (bde_verify::csabase::cast_ptr<clang::SwitchStmt> switchStmt = *it) {
                checkSwitch(analyser, switchStmt.get());
            }
        }
    }
    else {
        llvm::errs() << "body is not a component statement?\n";
    }
}

static bde_verify::csabase::RegisterCheck register_check(check_name, &check);
