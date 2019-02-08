// csamisc_contiguousswitch.cpp                                       -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>
#include <csabase_analyser.h>
#include <csabase_cast_ptr.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_report.h>
#include <csabase_registercheck.h>
#include <csabase_visitor.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/Support/Casting.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <algorithm>
#include <string>
#include <vector>

namespace csabase { class PPObserver; }

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("contiguous-switch");

// ----------------------------------------------------------------------------

namespace
{

struct data
    // This class contains data needed for checking test-driver switch
    // statements.
{
};

struct report : Report<data> {
    // This class contains methods and callbacks for checking test-driver
    // switch statements.

    INHERIT_REPORT_CTOR(report, Report, data);

    bool getValue(Expr const *expr, long& value);
        // Return 'true' if the specified 'expr' can be evaluated as an integer
        // constant and set the specified 'value' to that constant.  Otherwise
        // return 'false' and leave 'value' unchanged.

    void operator()(FunctionDecl const *decl);
        // Callback for encountering the specified 'decl'.

    void check_switch(SwitchStmt const *stmt);
        // Check the specified 'stmt' for test-driver correctness.

    BreakStmt const* last_break(Stmt::const_child_range s);
        // If the last statement in the specified range 's' is a 'break'
        // statement (including within nested compund statements), return it,
        // otherwise return null.
};

bool report::getValue(Expr const *expr, long& value)
{
    llvm::APSInt result;
    if (expr->EvaluateAsInt(result, *a.context())) {
        value = result.getSExtValue();
        return true;
    }
    return false;
}

void report::operator()(FunctionDecl const *decl)
{
    if (!decl->isMain() || !decl->hasBody() || !a.is_test_driver()) {
        return;                                                       // RETURN
    }
    if (cast_ptr<CompoundStmt> body = decl->getBody()) {
        for (CompoundStmt::const_body_iterator it  = body->body_begin(),
                                               end = body->body_end();
             it != end; ++it) {
            if (cast_ptr<SwitchStmt> switchStmt = *it) {
                check_switch(switchStmt.get());
            }
        }
    }
}

void report::check_switch(SwitchStmt const* stmt)
{
    CompoundStmt const* body = llvm::dyn_cast<CompoundStmt>(stmt->getBody());
    if (!body) {
        return;                                                       // RETURN
    }

    bool saw_break = true;
    bool multi = false;
    for (Stmt::const_child_iterator b = body->child_begin(),
                                    e = body->child_end(); b != e; ++b) {
        SwitchCase const* sc = llvm::dyn_cast<SwitchCase>(*b);
        BreakStmt  const* bs = llvm::dyn_cast<BreakStmt>(*b);
        NullStmt   const* ns = llvm::dyn_cast<NullStmt>(*b);

        if (sc) {
            if (!saw_break) {
                a.report(sc, check_name, "MB01",
                         "Missing `break;` before this case");
            }
            Stmt         const* sub = sc->stripLabelLikeStatements();
            CompoundStmt const* cs = llvm::dyn_cast<CompoundStmt>(sub);
            BreakStmt    const* bs = llvm::dyn_cast<BreakStmt>(sub);
            NullStmt     const* ns = llvm::dyn_cast<NullStmt>(sub);
            if (cs) {
                saw_break = 0 != last_break(cs->children());
                multi = false;
            } else if (!ns) {
                saw_break = 0 != bs;
                if (!saw_break) {
                    a.report(sub, check_name, "CS01",
                             "Test code should be within compound "
                             "(brace-enclosed) statement");
                }
                multi = true;
            }
        }
        else if (bs) {
            saw_break = true;
            multi = false;
        }
        else if (!ns) {
            if (!multi) {
                a.report(*b, check_name, "CS02",
                         "Test code should be within single compound "
                         "(brace-enclosed) statement");
                multi = true;
            }
            CompoundStmt const* cs = llvm::dyn_cast<CompoundStmt>(*b);
            if (cs) {
                saw_break = 0 != last_break(cs->children());
            } else {
                saw_break = false;
            }
        }
    }

    const DefaultStmt *ds = 0;
    typedef std::vector<SwitchCase const*>::const_iterator const_iterator;
    std::vector<SwitchCase const*> cases;
    for (SwitchCase const* sc(stmt->getSwitchCaseList());
         sc; sc = sc->getNextSwitchCase()) {
        cases.push_back(sc);
        if (!ds) {
            ds = llvm::dyn_cast<DefaultStmt>(sc);
        }
    }
    if (cases.empty()) {
        a.report(stmt, check_name, "ES01", "Empty switch statement");
        return;
    }
    std::reverse(cases.begin(), cases.end());
    const_iterator b = cases.begin();
    const_iterator e = cases.end();

    if (!ds) {
        a.report(stmt, check_name, "ED01",
                 "Switch doesn't end with `default:` label");
    } else if (!llvm::dyn_cast<DefaultStmt>(cases.back())) {
        if (llvm::dyn_cast<DefaultStmt>(cases.front())) {
            a.report(ds, check_name, "SD01",
                     "Switch starts with `default:` label");
            ++b;
        } else {
            a.report(ds, check_name, "MD01",
                     "`default:` label in the middle of labels");
        }
    } else {
        --e;
    }

    long previous_label = 0;
    long min_label = 0;
    const CaseStmt *min_case = 0;

    for (const_iterator it = b; it != e; ++it) {
        if (CaseStmt const* cs = llvm::dyn_cast<CaseStmt>(*it)) {
            long value;
            if (!getValue(cs->getLHS(), value)) {
                continue;
            }
            if (value > 0 && (min_label == 0 || value < min_label)) {
                min_label = value;
                min_case = cs;
            }
            if (previous_label > 0 &&
                value > 0 &&
                previous_label - 1 != value) {
                a.report(cs, check_name, "LO01",
                         "Case label out of order: "
                         "previous=%0 value=%1")
                    << previous_label
                    << value;
            }
            previous_label = value;
            if ((value == 0) != (it == b)) {
                a.report(cs, check_name, "SZ01",
                         "Switch should start with `case 0:`");
            }
            if (value == 0) {
                if (!llvm::dyn_cast<SwitchCase>(cs->getSubStmt())) {
                    a.report(cs, check_name, "ZF02",
                             "`case 0:` should simply fall through "
                             "to next case");
                } else {
                    continue;
                }
            }
        }
    }
    if (min_label > 1) {
        a.report(min_case, check_name, "SM01",
                 "Switch missing case values below %0")
            << min_label;
    }
}

BreakStmt const* report::last_break(Stmt::const_child_range r)
{
    BreakStmt const* brk = 0;
    for (const Stmt *s : r) {
        if (llvm::dyn_cast<CompoundStmt>(s)) {
            // Recurse into simple compound statements.
            brk = last_break(s->children());
        } else {
            // Try to cast each statement to a BreakStmt. Therefore 'brk'
            // will only be non-zero if the final statement is a 'break'.
            brk = llvm::dyn_cast<BreakStmt>(s);
        }
    }
    return brk;
}

// ----------------------------------------------------------------------------

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver&)
    // Create callbacks within the specified 'visitor' using the specified
    // 'analyser' which will be invoked at various translation steps.
{
    visitor.onFunctionDecl += report(analyser);
}

}  // close anonymous namespace

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
