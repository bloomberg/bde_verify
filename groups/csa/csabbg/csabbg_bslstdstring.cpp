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
                            pointee(hasDeclaration(
                            namedDecl(allOf(unless(matchesName("string_view")),
                                            matchesName("::(native_)?std(::__cxx11)?::basic_string"))))
            )))))))).bind("e"))),
        &m1);

    OnMatch<> m2([&](const BoundNodes &nodes) {
        auto e = nodes.getNodeAs<Expr>("e");
        a.report(e, check_name, "ST02",
                 "Converting bsl::string to std::string");
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(
            cxxMemberCallExpr(
                on(hasType(typedefDecl(matchesName("::bsl::string")))),
                callee(cxxMethodDecl(allOf(unless(matchesName("operator basic_string_view")),
                                           matchesName("operator basic_string")))))
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
