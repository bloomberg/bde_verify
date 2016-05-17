// csamisc_donotuseendl.cpp                                           -*-C++-*-

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

static std::string const check_name("do-not-use-endl");

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
    if (a.is_test_driver()) {
        return;                                                       // RETURN
    }

    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        const auto *c = nodes.getNodeAs<CXXOperatorCallExpr>("c");
        a.report(c->getOperatorLoc(), check_name, "NE01",
                 "Prefer ... << '\\n', and ... << flush if needed");
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(cxxOperatorCallExpr(
            hasOverloadedOperatorName("<<"),
            hasAnyArgument(ignoringImpCasts(declRefExpr(
                to(namedDecl(hasName("std::endl")))
            )))
        ).bind("c"))), &m1);
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
