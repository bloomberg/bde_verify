// csatr_componentprefix.cpp                                          -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/Support/Casting.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("component-prefix");

// ----------------------------------------------------------------------------

static bool wrong_prefix(Analyser& analyser, const NamedDecl* named)
{
    std::string package_prefix = analyser.package() + "_";
    std::string name = named->getNameAsString();
    if (name.find(package_prefix) != 0) {
        name = package_prefix + name;
    }
    return 0 != to_lower(name).find(analyser.component()) &&
           0 != to_lower(named->getQualifiedNameAsString())
                    .find(to_lower(
                         analyser.config()->toplevel_namespace() + "::" +
                         analyser.component() + "::"));
}

// ----------------------------------------------------------------------------

static void component_prefix(Analyser& analyser, Decl const *decl)
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
    if (   !analyser.is_global_package()
        && !analyser.is_global_name(named)
        && (   named->hasLinkage()
            || (   llvm::dyn_cast<TypedefDecl>(named)
                && name.find(analyser.package() + "_") != 0
               ))
        && !llvm::dyn_cast<NamespaceDecl>(named)
        && !llvm::dyn_cast<UsingDirectiveDecl>(named)
        && !llvm::dyn_cast<UsingDecl>(named)
        && analyser.is_component_header(decl)
        && wrong_prefix(analyser, named)
        && (!llvm::dyn_cast<RecordDecl>(named)
            || llvm::dyn_cast<RecordDecl>(named)->isCompleteDefinition()
            )
        && (   !fd
            || (!fd->isOverloadedOperator()
                 && name != "swap"
                 && name != "debugprint"
                 && !analyser.is_ADL_candidate(fd))
            )
        && !llvm::dyn_cast<ClassTemplateSpecializationDecl>(decl)
        && !llvm::dyn_cast<ClassTemplatePartialSpecializationDecl>(decl)
        && !(   llvm::dyn_cast<CXXRecordDecl>(decl)
             && llvm::dyn_cast<CXXRecordDecl>(decl)->
                                                   getDescribedClassTemplate())
        ) {
        analyser.report(decl, check_name, "CP01",
                        "Globally visible identifier '%0' "
                        "without component prefix")
            << named->getQualifiedNameAsString();
    }
}

// ----------------------------------------------------------------------------

static RegisterCheck check(check_name, &component_prefix);

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
