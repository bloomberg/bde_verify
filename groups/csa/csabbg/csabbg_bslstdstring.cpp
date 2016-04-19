// csabbg_bslstdstring.cpp                                            -*-C++-*-

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

static std::string const check_name("bsl-std-string");

// -----------------------------------------------------------------------------

namespace
{

struct data
{
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
};

void report::operator()()
{
    auto tu = a.context()->getTranslationUnitDecl();
    MatchFinder mf;

    OnMatch<> m1([&](const BoundNodes &nodes) {
        auto e = nodes.getNodeAs<Expr>("e");
        a.report(e, check_name, "ST01",
                 "Converting std::string to bsl::string");
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            cxxConstructExpr(
                hasDeclaration(cxxConstructorDecl(
                    matchesName("::bsl::basic_string<"),
                    hasParameter(
                        0,
                        parmVarDecl(hasType(referenceType(
                            pointee(hasDeclaration(namedDecl(matchesName(
                                "::(native_)?std::basic_string")))))))))))
                .bind("e"))),
        &m1);

    OnMatch<> m2([&](const BoundNodes &nodes) {
        auto e = nodes.getNodeAs<Expr>("e");
        a.report(e, check_name, "ST02",
                 "Converting bsl::string to std::string");
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            cxxMemberCallExpr(
                on(hasType(recordDecl(matchesName("::bsl::basic_string")))),
                callee(cxxMethodDecl(matchesName("operator basic_string"))))
                .bind("e"))),
        &m2);

    mf.match(*tu, *a.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
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
