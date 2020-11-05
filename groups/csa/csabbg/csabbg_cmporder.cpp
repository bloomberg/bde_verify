// csabbg_cmporder.cpp                                                -*-C++-*-

#include <clang/AST/Stmt.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <set>

using namespace clang;
using namespace csabase;

// -----------------------------------------------------------------------------

static std::string const check_name("comparison-order");

// -----------------------------------------------------------------------------

namespace
{

struct data
{
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    bool carefullyIsModifiable(const Expr *e);
    bool carefullyIsEvaluatable(const Expr *e);

    void operator()(const BinaryOperator *op);
};

bool report::carefullyIsModifiable(const Expr *e)
    // We want to just call isModifiableLvalue, but that triggers an assertion
    // on dependent expressions.
{
    return !e->isValueDependent() &&
           !e->isTypeDependent() &&
           //!e->isInstantiationDependent() &&
           e->isModifiableLvalue(*a.context()) == Expr::MLV_Valid;
}

bool report::carefullyIsEvaluatable(const Expr *e)
    // We want to just call isEvaluatable, but that triggers an assertion on
    // dependent expressions.
{
    return !e->isValueDependent() &&
           !e->isTypeDependent() &&
           //!e->isInstantiationDependent() &&
           e->isEvaluatable(*a.context());
}

void report::operator()(const BinaryOperator *op)
{
    if (!op->getOperatorLoc().isMacroID() && op->isEqualityOp()) {
        auto lhs = op->getLHS()->IgnoreParenImpCasts();
        auto rhs = op->getRHS()->IgnoreParenImpCasts();
        const char *tag = 0;
        if (carefullyIsModifiable(lhs) && !carefullyIsModifiable(rhs)) {
            tag = "CO01";
            auto report = a.report(op->getOperatorLoc(), check_name, tag,
                     "Non-modifiable operand should be on the left");
            report << op->getSourceRange();
        }
        else if (!carefullyIsEvaluatable(lhs) && carefullyIsEvaluatable(rhs)) {
            tag = "CO02";
            auto report = a.report(op->getOperatorLoc(), check_name, tag,
                     "Constant-expression operand should be on the left");
            report << op->getSourceRange();
        }
        if (tag) {
            SourceRange lr(
                a.get_full_range(op->getLHS()->getSourceRange()));
            SourceRange rr(
                a.get_full_range(op->getRHS()->getSourceRange()));
            llvm::StringRef rop =
                op->getOpcodeStr(op->reverseComparisonOp(op->getOpcode()));
            SourceRange opr(a.get_full_range(
                SourceRange(op->getOperatorLoc(), op->getOperatorLoc())));
            llvm::StringRef wl = a.get_source(
                SourceRange(lr.getEnd(), opr.getBegin()), true);
            if (!wl.size() || wl[0] != ' ') {
                wl = "";
            }
            llvm::StringRef wr = a.get_source(
                SourceRange(opr.getEnd(), rr.getBegin()), true);
            if (!wr.size() || wr[0] != ' ') {
                wr = "";
            }
            std::string rev = a.get_source(rr, true).str() +
                              wr.str() + rop.str() + wl.str() +
                              a.get_source(lr, true).str();
            auto report = a.report(op->getOperatorLoc(), check_name, tag,
                     "Replace with %0", false, DiagnosticIDs::Note);

            report << rev
                   << op->getSourceRange();

            a.ReplaceText(a.get_full_range(op->getSourceRange()), rev);
        }
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onBinaryOperator += report(analyser);
}

}  // close anonymous namespace

// -----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
