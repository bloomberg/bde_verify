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
        Expr::EvalResult result;
        return Node.EvaluateAsInt(result, Finder->getASTContext());
    }
    AST_MATCHER_P2(Expr, inRange, long long, low, long long, high) {
        Expr::EvalResult result;
        return Node.EvaluateAsInt(result, Finder->getASTContext()) &&
               result.Val.getInt().getSExtValue() >= low &&
               result.Val.getInt().getSExtValue() <= high;
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
    INHERIT_REPORT_CTOR(report, Report, data);

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
        !is.count(f->getName().str())) {
        return;                                                       // RETURN
    }
    const DeclContext *dc = f->getParent();
    if (dc->isNamespace()) {
        const NamespaceDecl *ns = llvm::dyn_cast<NamespaceDecl>(dc);
        if (ns->isAnonymousNamespace() ||
            !a.is_standard_namespace(ns->getName().str())) {
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
