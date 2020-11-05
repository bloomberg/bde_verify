// csatr_componentprefix.cpp                                          -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/Support/Casting.h>
#include <set>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("component-prefix");

// ----------------------------------------------------------------------------

namespace {

struct data
{
    typedef std::set<std::pair<Location, std::string>> BadGlobals;
    BadGlobals d_badglobals;
};

struct report : Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    bool wrong_prefix(const NamedDecl* named);

    void operator()();
    void operator()(const Decl *decl);
};

bool report::wrong_prefix(const NamedDecl *named)
{
    std::string package_prefix = a.package() + "_";
    std::string name = named->getNameAsString();
    if (name.find(package_prefix) != 0) {
        name = package_prefix + name;
    }
    return 0 != to_lower(name).find(a.component()) &&
           0 != to_lower(named->getQualifiedNameAsString())
                    .find(to_lower(a.config()->toplevel_namespace() +
                                   "::" + a.component() + "::"));
}

void report::operator()()
{
    for (const auto &bg : d.d_badglobals) {
        auto report = a.report(bg.first.location(), check_name, "CP01",
                 "Globally visible identifier '%0' without component prefix");
        report << bg.second;
    }
}

void report::operator()(const Decl *decl)
{
    const DeclContext *dc = decl->getDeclContext();
    if (dc->isClosure() || dc->isFunctionOrMethod() || dc->isRecord()) {
        return;                                                       // RETURN
    }

    FunctionDecl const* fd = llvm::dyn_cast<FunctionDecl>(decl);
    if (fd && fd->isExternC()) {
        return;
    }
    VarDecl const* vd = llvm::dyn_cast<VarDecl>(decl);
    if (vd && vd->isExternC()) {
        return;
    }
    NamedDecl const* named(llvm::dyn_cast<NamedDecl>(decl));
    if (!named) {
        return;
    }
    if (FunctionTemplateDecl const* ftd =
            llvm::dyn_cast<FunctionTemplateDecl>(decl)) {
        fd = ftd->getTemplatedDecl();
    }
    std::string const& name(named->getNameAsString());
    NamedDecl const *member_check = named;
    if (EnumDecl const *ed = llvm::dyn_cast<EnumDecl>(dc)) {
        // Handle both scoped and unscoped enumerators.
        member_check = ed;
    }
    if (member_check->isCXXClassMember()) {
        return;
    }
    auto rd = llvm::dyn_cast<CXXRecordDecl>(decl);
    if (   !a.is_global_package()
        && !a.is_global_name(named)
        && (   named->hasLinkage()
            || (   llvm::dyn_cast<TypedefDecl>(named)
                && name.find(a.package() + "_") != 0
               ))
        && !llvm::dyn_cast<NamespaceDecl>(named)
        && !llvm::dyn_cast<UsingDirectiveDecl>(named)
        && !llvm::dyn_cast<UsingDecl>(named)
        && a.is_component_header(decl)
        && wrong_prefix(named)
        && !(rd && !rd->isCompleteDefinition())
        && !(rd && rd->getTemplateInstantiationPattern())
        && (   !fd
            || (!fd->isOverloadedOperator()
                 && name != "swap"
                 && name != "debugprint"
                 && !a.is_ADL_candidate(fd))
            )
        && !llvm::dyn_cast<ClassTemplateSpecializationDecl>(decl)
        && !llvm::dyn_cast<ClassTemplatePartialSpecializationDecl>(decl)
        ) {
            d.d_badglobals.emplace(Location(m, decl->getLocation()),
                                   named->getQualifiedNameAsString());
    }
}

void subscribe(Analyser& analyser, Visitor& visitor, PPObserver& observer)
    // Hook up the callback functions.
{
    visitor.onDecl += report(analyser);
    analyser.onTranslationUnitDone += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck check(check_name, &subscribe);

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
