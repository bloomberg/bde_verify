// csamisc_swapusing.cpp                                              -*-C++-*-

#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
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
    bool d_bsl;

    data();
};

data::data() : d_bsl(false)
{
}

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    // Preprocessor FileChanged callback
    void operator()(SourceLocation                now,
                    PPCallbacks::FileChangeReason reason,
                    SrcMgr::CharacteristicKind    type,
                    FileID                        prev);

    // TranslationUnitDone callback
    void operator()();
};

void report::operator()(SourceLocation                now,
                        PPCallbacks::FileChangeReason reason,
                        SrcMgr::CharacteristicKind    type,
                        FileID                        prev)
{
    if (!d.d_bsl) {
        FileName file(m.getPresumedLoc(now).getFilename());
        d.d_bsl = file.name() == "bsl_utility.h" ||
                  file.name() == "bsl_algorithm.h";
    }
}

void report::operator()()
{
    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        auto report = a.report(nodes.getNodeAs<CallExpr>("c"), check_name, "SU01",
                 "Prefer 'using %0::swap; swap(...);'");
        report << (d.d_bsl ? "bsl" : "std");
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(callExpr(
            callee(namedDecl(hasName("std::swap"))),
            callee(stmt(expr(ignoringImpCasts(declRefExpr(
                qualifier(specifiesNamespace(anything()))
            ))))),
            unless(hasAnyArgument(hasType(qualType(builtinType()))))
        ).bind("c"))), &m1);
    mf.match(*a.context()->getTranslationUnitDecl(), *a.context());
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onPPFileChanged       += report(analyser);
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
