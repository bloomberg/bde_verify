// csastil_uppernames.cpp                                             -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Preprocessor.h>
#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <set>
#include <string>
#include <utility>

namespace csabase { class PPObserver; }
namespace csabase { class Visitor; }

using namespace clang;
using namespace clang::ast_matchers;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("upper-case-names");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    std::set<NamedDecl const*> d_decls;
    std::vector<SourceRange> d_omit_internal_deprecated;
};

struct report :  Report<data>
    // Complain about upper-case only variables and constants.
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void match_has_name(const BoundNodes& nodes);
        // Find named declarations.

    void operator()();
        // Callback invoked at end of translation unit.

    void operator()(SourceLocation,
                    SourceRange,
                    PPCallbacks::ConditionValueKind,
                    SourceLocation);
        // Callback for #elif.

    void operator()(SourceLocation, const Token &, const MacroDefinition &);
        // Callback for #ifndef.

    void operator()(SourceLocation, SourceLocation);
        // Callback for #else/#endif.
};

internal::DynTypedMatcher has_name_matcher()
    // Return an AST matcher which looks for named declarations.
{
    return decl(forEachDescendant(namedDecl().bind("decl")));
}

void report::match_has_name(const BoundNodes& nodes)
{
    NamedDecl const* decl = nodes.getNodeAs<NamedDecl>("decl");
    if (!d_data.d_decls.insert(decl).second) {
        return;                                                       // RETURN
    }
    if (decl->getLocation().isMacroID()) {
        return;                                                       // RETURN
    }
    if (!d_analyser.is_component(decl)) {
        return;                                                       // RETURN
    }
    if (llvm::dyn_cast<TemplateTypeParmDecl>(decl) ||
        llvm::dyn_cast<TemplateTemplateParmDecl>(decl) ||
        llvm::dyn_cast<NonTypeTemplateParmDecl>(decl)) {
        return;                                                       // RETURN
    }
    if (FunctionDecl const* func = llvm::dyn_cast<FunctionDecl>(decl)) {
        if (func->isDefaulted()) {
            return;                                                   // RETURN
        }
    }
    if (RecordDecl const* record = llvm::dyn_cast<RecordDecl>(decl)) {
        if (record->isInjectedClassName()) {
            return;                                                   // RETURN
        }
    }

    IdentifierInfo const* ii = decl->getIdentifier();
    if (ii) {
        llvm::StringRef name = ii->getName();
        if (name.find_first_of("abcdefghijklmnopqrstuvwxyz") == name.npos &&
            name.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != name.npos &&
            name.find(llvm::StringRef(d_analyser.component()).upper()) ==
                name.npos) {
            bool omit = false;
            for (auto i : d.d_omit_internal_deprecated) {
                if (m.isBeforeInTranslationUnit(i.getBegin(),
                                                decl->getLocation()) &&
                    m.isBeforeInTranslationUnit(decl->getLocation(),
                                                i.getEnd())) {
                    omit = true;
                    break;
                }
            }
            if (!omit) {
                d_analyser.report(decl, check_name, "UC01",
                                  "Name should not be all upper-case");
            }
        }
    }

    if (TemplateDecl const* tplt = llvm::dyn_cast<TemplateDecl>(decl)) {
        d_data.d_decls.insert(tplt->getTemplatedDecl());
    }
}

void report::operator()()
{
    if (!d_analyser.is_test_driver()) {
        MatchFinder mf;
        OnMatch<report, &report::match_has_name> m1(this);
        mf.addDynamicMatcher(has_name_matcher(), &m1);
        mf.match(*d_analyser.context()->getTranslationUnitDecl(),
                 *d_analyser.context());
    }
}

// Ifndef
void report::operator()(SourceLocation         loc,
                        const Token&           macro,
                        const MacroDefinition& md)
{
    if (macro.getIdentifierInfo()->getName() ==
        "BDE_OMIT_INTERNAL_DEPRECATED") {
        d.d_omit_internal_deprecated.emplace_back(loc);
    }
}

// Else
// Endif
void report::operator()(SourceLocation loc, SourceLocation ifloc)
    // Callback for #else/#endif.
{
    for (auto &i : d.d_omit_internal_deprecated) {
        if (i.getBegin() == ifloc && i.getEnd() == ifloc) {
            i.setEnd(loc);
        }
    }
}

// Elif
void report::operator()(SourceLocation                  loc,
                        SourceRange                     condition,
                        PPCallbacks::ConditionValueKind kind,
                        SourceLocation                  ifloc)
{
    (*this)(loc, ifloc);
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);

    observer.onPPIfndef += report(analyser, observer.e_Ifndef);
    observer.onPPElif   += report(analyser, observer.e_Elif);
    observer.onPPElse   += report(analyser, observer.e_Else);
    observer.onPPEndif  += report(analyser, observer.e_Endif);
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
