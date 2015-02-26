// csamisc_charclassrange.cpp                                         -*-C++-*-

#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <string>
#include <unordered_set>

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

static std::string const check_name("char-classification-range");

namespace clang {
namespace ast_matchers {
    AST_MATCHER(Expr, known) {
        llvm::APSInt value;
        return Node.EvaluateAsInt(value, Finder->getASTContext());
    }
    AST_MATCHER_P2(Expr, inRange, long long, low, long long, high) {
        llvm::APSInt value;
        return Node.EvaluateAsInt(value, Finder->getASTContext()) &&
               value.getSExtValue() >= low &&
               value.getSExtValue() <= high;
    }
}
}

namespace
{

struct data
{
};

struct report : Report<data>
{
    using Report<data>::Report;

    void operator()(const CallExpr *expr);
};

void report::operator()(const CallExpr *expr)
{
    static std::unordered_set<std::string> is{
        "isalnum", "isalpha", "iscntrl", "isdigit", "islower", "isgraph",
        "isprint", "ispunct", "isspace", "isupper", "isxdigit"
    };
    const FunctionDecl *f = expr->getDirectCallee();
    if (!f ||
        f->getNumParams() != 1 ||
        !f->getIdentifier() ||
        !is.count(f->getName())) {
        return;                                                       // RETURN
    }
    const DeclContext *dc = f->getParent();
    if (dc->isNamespace()) {
        const NamespaceDecl *ns = llvm::dyn_cast<NamespaceDecl>(dc);
        if (ns->isAnonymousNamespace() ||
            !a.is_standard_namespace(ns->getName())) {
            return;                                                   // RETURN
        }
    }
    else if (!dc->isExternCContext()) {
        return;                                                       // RETURN
    }

    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        a.report(nodes.getNodeAs<CallExpr>("1")->getArg(0), check_name, "ISC01",
                 "Sign extension may cause unexpected behavior");
    });
    mf.addDynamicMatcher(
        callExpr(
            hasArgument(0, allOf(
                ignoringImpCasts(hasType(asString("char"))),
                unless(known())
            ))
        ).bind("1"), &m1);
    OnMatch<> m2([&](const BoundNodes &nodes) {
        a.report(nodes.getNodeAs<CallExpr>("2")->getArg(0), check_name, "ISC02",
                 "Sign extension may cause unexpected behavior");
    });
    mf.addDynamicMatcher(
        callExpr(
            hasArgument(0, allOf(
                ignoringImpCasts(hasType(asString("char"))),
                known(),
                unless(inRange(0, 255))
            ))
        ).bind("2"), &m2);
    OnMatch<> m3([&](const BoundNodes &nodes) {
        a.report(nodes.getNodeAs<CallExpr>("3")->getArg(0), check_name, "ISC03",
                 "Out-of-range value may cause unexpected behavior");
    });
    mf.addDynamicMatcher(
        callExpr(
            hasArgument(0, allOf(
                unless(ignoringImpCasts(hasType(asString("char")))),
                known(),
                unless(inRange(-1, 255))
            ))
        ).bind("3"), &m3);
    mf.match(*expr, *a.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onCallExpr += report(analyser);
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
