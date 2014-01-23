// csatr_componentprefix.cpp                                          -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_format.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <sstream>
#include <cctype>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("component-prefix");

// ----------------------------------------------------------------------------

static bool
wrong_prefix(cool::csabase::Analyser&  analyser,
             const clang::NamedDecl   *named)
{
    std::string package_prefix = analyser.package() + "_";
    std::string name = named->getNameAsString();
    if (name.find(package_prefix) != 0) {
        name = package_prefix + name;
    }
    return 0 != cool::csabase::to_lower(name).find(analyser.component()) &&
           0 != cool::csabase::to_lower(named->getQualifiedNameAsString())
                    .find(cool::csabase::to_lower(
                         analyser.config()->toplevel_namespace() + "::" +
                         analyser.component() + "::"));
}

// ----------------------------------------------------------------------------

static void
component_prefix(cool::csabase::Analyser&  analyser,
                 clang::Decl const        *decl)
{
    const clang::DeclContext *dc = decl->getDeclContext();
    if (dc->isClosure() || dc->isFunctionOrMethod() || dc->isRecord()) {
        return;                                                       // RETURN
    }

    clang::NamedDecl const* named(llvm::dyn_cast<clang::NamedDecl>(decl));
    clang::FunctionDecl const* fd = llvm::dyn_cast<clang::FunctionDecl>(decl);
    if (clang::FunctionTemplateDecl const* ftd =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
        fd = ftd->getTemplatedDecl();
    }
    std::string const& name(named ? named->getNameAsString() : std::string());
    if (   !name.empty()
        && !analyser.is_global_package()
        && !named->isCXXClassMember()
        && (   named->hasLinkage()
            || (   llvm::dyn_cast<clang::TypedefDecl>(named)
                && name.find(analyser.package() + "_") != 0
               ))
        && !llvm::dyn_cast<clang::NamespaceDecl>(named)
        && !llvm::dyn_cast<clang::UsingDirectiveDecl>(named)
        && !llvm::dyn_cast<clang::UsingDecl>(named)
        && analyser.is_component_header(decl)
        && wrong_prefix(analyser, named)
        && (!llvm::dyn_cast<clang::RecordDecl>(named)
            || llvm::dyn_cast<clang::RecordDecl>(named)->isCompleteDefinition()
            )
        && (   !fd
            || (!fd->isOverloadedOperator()
                 && name != "swap"
                 && name != "debugprint"
                 && !analyser.is_ADL_candidate(fd))
            )
        && !llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)
        && !llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl)
        && !(   llvm::dyn_cast<clang::CXXRecordDecl>(decl)
             && llvm::dyn_cast<clang::CXXRecordDecl>(decl)->
                                                   getDescribedClassTemplate())
        ) {
        analyser.report(decl, check_name, "TR05",
                        "Globally visible identifier '%0' "
                        "without component prefix")
            << named->getQualifiedNameAsString();
    }
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name, &component_prefix);
