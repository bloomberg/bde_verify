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
    if (!a.is_component(op)) {
        return;                                                       // RETURN
    }

    if (!op->getOperatorLoc().isMacroID() && op->isEqualityOp()) {
        auto lhs = op->getLHS()->IgnoreParenImpCasts();
        auto rhs = op->getRHS()->IgnoreParenImpCasts();
        const char *tag = 0;
        if (carefullyIsModifiable(lhs) && !carefullyIsModifiable(rhs)) {
            tag = "CR01";
            a.report(op->getOperatorLoc(), check_name, tag,
                     "Non-modifiable operand should be on the left")
                << op->getSourceRange();
        }
        else if (!carefullyIsEvaluatable(lhs) && carefullyIsEvaluatable(rhs)) {
            tag = "CR02";
            a.report(op->getOperatorLoc(), check_name, tag,
                     "Constant-expression operand should be on the left")
                << op->getSourceRange();
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
            a.report(op->getOperatorLoc(), check_name, tag,
                     "Replace with %0", false, DiagnosticIDs::Note)
                << rev
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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
