// csastil_templatetypename.cpp                                       -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/ExprCXX.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Regex.h>
#include <stddef.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <string>

namespace csabase { class PPObserver; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang;
using namespace clang::ast_matchers;

// ----------------------------------------------------------------------------

static std::string const check_name("template-typename");

// ----------------------------------------------------------------------------

namespace
{

struct report
    // Complain about uses of 'typename' instead of 'class'.
{
    report(Analyser& analyser);
        // Create an object of this type using the specified analyser.

    void match_has_template_parameters(const BoundNodes& nodes);
        // Callback function invoked when the specified 'nodes' contain an
        // object which may have template parameters.

    void checkTemplateParameters(TemplateParameterList const* parms);
        // Complain about the specified template 'parms' that use 'typename'
        // instead of 'class'.

    void operator()();
        // Callback invoked at end of translation unit.

    Analyser& d_analyser;
};

report::report(Analyser& analyser)
: d_analyser(analyser)
{
}

internal::DynTypedMatcher has_template_parameters_matcher()
    // Return an AST matcher which looks for things that might have template
    // parameters.  We could be more restrictive than just accepting any 'decl'
    // but that would just push the same work we do in the callback elsewhere.
{
    return decl(eachOf(forEachDescendant(decl().bind("decl")),
                       forEachDescendant(lambdaExpr().bind("lambda"))));
}

void report::match_has_template_parameters(const BoundNodes& nodes)
{
    if (TemplateDecl const* decl = nodes.getNodeAs<TemplateDecl>("decl")) {
        checkTemplateParameters(decl->getTemplateParameters());
    }
    if (DeclaratorDecl const* decl = nodes.getNodeAs<DeclaratorDecl>("decl")) {
        unsigned n = decl->getNumTemplateParameterLists();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(decl->getTemplateParameterList(i));
        }
    }
    if (TagDecl const* decl = nodes.getNodeAs<TagDecl>("decl")) {
        unsigned n = decl->getNumTemplateParameterLists();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(decl->getTemplateParameterList(i));
        }
    }
    if (ClassTemplatePartialSpecializationDecl const* decl =
            nodes.getNodeAs<ClassTemplatePartialSpecializationDecl>("decl")) {
        checkTemplateParameters(decl->getTemplateParameters());
    }
    if (VarTemplatePartialSpecializationDecl const* decl =
            nodes.getNodeAs<VarTemplatePartialSpecializationDecl>("decl")) {
        checkTemplateParameters(decl->getTemplateParameters());
    }
    if (CXXRecordDecl const* decl = nodes.getNodeAs<CXXRecordDecl>("decl")) {
        checkTemplateParameters(decl->getGenericLambdaTemplateParameterList());
    }
    if (FriendDecl const* decl = nodes.getNodeAs<FriendDecl>("decl")) {
        unsigned n = decl->getFriendTypeNumTemplateParameterLists();
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(
                decl->getFriendTypeTemplateParameterList(i));
        }
    }
    if (TemplateTemplateParmDecl const* decl =
            nodes.getNodeAs<TemplateTemplateParmDecl>("decl")) {
        unsigned n = decl->isExpandedParameterPack() ?
                         decl->getNumExpansionTemplateParameters() :
                         0;
        for (unsigned i = 0; i < n; ++i) {
            checkTemplateParameters(decl->getExpansionTemplateParameters(i));
        }
    }
    if (LambdaExpr const* expr = nodes.getNodeAs<LambdaExpr>("lambda")) {
        checkTemplateParameters(expr->getTemplateParameterList());
    }
}

void report::operator()()
{
    MatchFinder mf;
    OnMatch<report, &report::match_has_template_parameters> m1(this);
    mf.addDynamicMatcher(has_template_parameters_matcher(), &m1);
    mf.match(*d_analyser.context()->getTranslationUnitDecl(),
             *d_analyser.context());
}

static llvm::Regex all_upper("^[[:upper:]](_?[[:upper:][:digit:]]+)*\r*$");

void report::checkTemplateParameters(TemplateParameterList const* parms)
{
    unsigned n = parms ? parms->size() : 0;
    for (unsigned i = 0; i < n; ++i) {
        auto parm = parms->getParam(i);
        auto nparm = llvm::dyn_cast<NonTypeTemplateParmDecl>(parm);
        auto tparm = llvm::dyn_cast<TemplateTypeParmDecl>(parm);
        auto ttparm = llvm::dyn_cast<TemplateTemplateParmDecl>(parm);
        if ((nparm || tparm || ttparm) &&
            !parm->getLocation().isMacroID() &&
            d_analyser.is_component(parm)) {
            if (tparm && tparm->wasDeclaredWithTypename()) {
                 d_analyser.report(tparm, check_name, "TY01",
                     "Template parameter uses 'typename' instead of 'class'");
                 SourceRange r = tparm->getSourceRange();
                 llvm::StringRef s = d_analyser.get_source(r);
                 size_t to = s.find("typename");
                 if (to != s.npos) {
                     d_analyser.ReplaceText(getOffsetRange(r, to, 8), "class");
                 }
            }
            if (parm->getIdentifier()) {
                if (parm->getName().size() == 1) {
                    d_analyser.report(parm, check_name, "TY02",
                        "Template parameter uses single-letter name");
                }
                if (!all_upper.match(parm->getName())) {
                    d_analyser.report(parm, check_name, "TY03",
                        "Template parameter is not in ALL_CAPS format");
                }
            }
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver&)
{
    analyser.onTranslationUnitDone += report(analyser);
}

}

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
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
