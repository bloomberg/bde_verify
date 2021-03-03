// csamisc_movablerefref.cpp                                          -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace clang { class ExprWithCleanups; }
namespace clang { class Stmt; }
namespace csabase { class PPObserver; }
namespace csabase { class Visitor; }

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("ref-to-movableref");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    std::string d_mr;
};

struct report : Report<data>
    // Callback object for detecting references to MovableRef.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()();
        // Callback for the end of the translation unit.

    void match_ref_to_movableref(const BoundNodes &nodes);
        // Callback when references to MovableRef are found.
};

internal::DynTypedMatcher ref_to_movableref_matcher()
    // Return an AST matcher which looks for function parameters that are
    // references to MovableRef.
{
    return decl(forEachDescendant(functionDecl(
        unless(isTemplateInstantiation()),
        forEachDescendant(parmVarDecl(hasType(referenceType(pointee(
            type(anything()).bind("pt")
        )))).bind("mrr"))
    )));
}

void report::match_ref_to_movableref(const BoundNodes &nodes)
{
    auto parm = nodes.getNodeAs<ParmVarDecl>("mrr");
    if (!a.is_component(parm)) {
        return;
    }
    auto type = nodes.getNodeAs<Type>("pt")->getUnqualifiedDesugaredType();
    auto rec = type->getAsCXXRecordDecl();
    if (!rec) {
        if (auto tst = type->getAs<TemplateSpecializationType>()) {
            if (auto td = tst->getTemplateName().getAsTemplateDecl()) {
                if (auto tdd = td->getTemplatedDecl()) {
                    rec = llvm::dyn_cast<CXXRecordDecl>(tdd);
                }
            }
        }
    }
    if (rec && rec->getQualifiedNameAsString() == d.d_mr) {
        d_analyser.report(parm, check_name, "MRR01",
            "MovableRef should be passed by value, not reference");
    }
}

void report::operator()()
{
    d.d_mr = a.config()->toplevel_namespace() + "::bslmf::MovableRef";
    if (a.lookup_name(d.d_mr)) {
        MatchFinder mf;
        OnMatch<report, &report::match_ref_to_movableref> m1(this);
        mf.addDynamicMatcher(ref_to_movableref_matcher(), &m1);
        mf.match(*a.context()->getTranslationUnitDecl(), *a.context());
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

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
