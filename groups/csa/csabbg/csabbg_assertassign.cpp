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
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
};

void report::operator()()
{
    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        auto parent = nodes.getNodeAs<FunctionDecl>("parent");
        auto negate = nodes.getNodeAs<UnaryOperator>("!");
        auto assign = nodes.getNodeAs<BinaryOperator>("=");
        if (m.getFileID(parent->getBeginLoc()) ==
            m.getFileID(m.getSpellingLoc(assign->getOperatorLoc())) &&
            negate->getOperatorLoc().isMacroID()) {
            auto report = a.report(assign->getOperatorLoc(), check_name, "AE01",
                     "Assignment appears as top-level macro condition");
            report << SourceRange(assign->getBeginLoc(),
                               assign->getEndLoc());
        }
    });
    mf.addDynamicMatcher(decl(forEachDescendant(unaryOperator(
        hasOperatorName("!"),
        hasUnaryOperand(ignoringParenImpCasts(
            binaryOperator(hasOperatorName("=")).bind("=")
        )),
        hasAncestor(decl(functionDecl()).bind("parent"))
    ).bind("!"))), &m1);
    mf.match(*a.context()->getTranslationUnitDecl(), *a.context());
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
