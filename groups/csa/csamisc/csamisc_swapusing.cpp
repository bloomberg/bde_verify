// csamisc_swapusing.cpp                                              -*-C++-*-

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

static std::string const check_name("swap-using");

namespace clang {
namespace ast_matchers {
    AST_MATCHER_P(DeclRefExpr, qualifier,
                  internal::Matcher<NestedNameSpecifier>, InnerMatcher) {
        if (const auto *Qualifier = Node.getQualifier()) {
            return InnerMatcher.matches(*Qualifier, Finder, Builder);
        }
        return false;
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

    void operator()(const CallExpr *call);
};

void report::operator()(const CallExpr *call)
{
    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        a.report(nodes.getNodeAs<CallExpr>("c"), check_name, "SU01",
                 "Prefer 'using std::swap; swap(...);'");
    });
    mf.addDynamicMatcher(
        callExpr(
            callee(namedDecl(hasName("std::swap"))),
            callee(stmt(expr(ignoringImpCasts(declRefExpr(
                qualifier(specifiesNamespace(anything()))
            ))))),
            unless(hasAnyArgument(hasType(qualType(builtinType()))))
        ).bind("c"), &m1);
    mf.match(*call, *a.context());
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
