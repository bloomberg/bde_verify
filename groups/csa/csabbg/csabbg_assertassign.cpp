// csabbg_assertassign.cpp                                            -*-C++-*-

#include <clang/AST/Stmt.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/ASTMatchers/ASTMatchersMacros.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

// -----------------------------------------------------------------------------

static std::string const check_name("assert-assign");

// -----------------------------------------------------------------------------

namespace
{

struct data
{
};

struct report : Report<data>
{
    using Report<data>::Report;

    void operator()(const UnaryOperator *op);
};

void report::operator()(const UnaryOperator *op)
{
    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        auto parent = nodes.getNodeAs<FunctionDecl>("parent");
        auto negate = nodes.getNodeAs<UnaryOperator>("!");
        auto assign = nodes.getNodeAs<BinaryOperator>("=");
        if (m.getFileID(parent->getLocStart()) ==
            m.getFileID(m.getSpellingLoc(assign->getOperatorLoc())) &&
            negate->getOperatorLoc().isMacroID()) {
            a.report(assign->getOperatorLoc(), check_name, "AE01",
                     "Assignment appears as top-level macro condition")
                << SourceRange(assign->getLocStart(),
                               assign->getLocEnd());
        }
    });
    mf.addDynamicMatcher(unaryOperator(
        hasOperatorName("!"),
        hasUnaryOperand(ignoringParenImpCasts(
            binaryOperator(hasOperatorName("=")).bind("=")
        )),
        hasAncestor(decl(functionDecl()).bind("parent"))
    ).bind("!"), &m1);
    mf.match(*op, *a.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onUnaryOperator += report(analyser);
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
