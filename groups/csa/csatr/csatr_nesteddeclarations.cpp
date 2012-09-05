// -*-c++-*- groups/csa/csatr/csatr_nesteddeclarations.cpp 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_format.h>
#include <csabase_registercheck.h>
#include <clang/AST/DeclTemplate.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("nested-declarations");

static void
check(cool::csabase::Analyser& analyser, clang::Decl const* decl)
{
    cool::csabase::Location location(analyser.get_location(decl));
    clang::NamedDecl const* named(llvm::dyn_cast<clang::NamedDecl>(decl));
    if (analyser.is_component(location.file()) && named) {
        if (llvm::dyn_cast<clang::NamespaceDecl>(decl)
            || llvm::dyn_cast<clang::UsingDirectiveDecl>(decl)) {
            // namespace declarations are permissible e.g. for forward declarations.
            return;
        }
        else if (clang::TagDecl const* tag = llvm::dyn_cast<clang::TagDecl>(decl)) {
            // Forward declarations are always permissible but definitions are not.
            if (!tag->isThisDeclarationADefinition()
                && analyser.is_component_header(location.file())) {
                return;
            }
        }
        else if (llvm::dyn_cast<clang::NamespaceAliasDecl>(decl)
                 && analyser.is_component_source(decl)) {
            return;
        }

        clang::DeclContext const* context(decl->getDeclContext());
        if (llvm::dyn_cast<clang::TranslationUnitDecl>(context)) {
            if (named->getNameAsString() != "main"
                && named->getNameAsString() != "RCSId") {
                analyser.report(decl, check_name, "TR04: declaration of '%0' at global scope", true)
                    << decl->getSourceRange()
                    << named->getNameAsString();
            }
            return;
        }

        clang::NamespaceDecl const* space(llvm::dyn_cast<clang::NamespaceDecl>(context));
        if (space
            && space->isAnonymousNamespace()
            && llvm::dyn_cast<clang::NamespaceDecl>(space->getDeclContext())) {
            space = llvm::dyn_cast<clang::NamespaceDecl>(space->getDeclContext());
        }
        if (space && named->getNameAsString() != std::string())
        {
            clang::NamespaceDecl const* outer(llvm::dyn_cast<clang::NamespaceDecl>(space->getDeclContext()));
            if ((space->getNameAsString() != analyser.package()
                 || !outer
                 || outer->getNameAsString() != analyser.config()->toplevel_namespace())
                && (false //-dk:TODO !analyser.package_legacy()
                    || space->getNameAsString() != analyser.config()->toplevel_namespace()
                    || named->getNameAsString().find(analyser.package() + '_') != 0
                    )
                && !llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl)
                && !llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)
                )
            {
                //-dk:TODO check if this happens in the correct namespace
                analyser.report(decl, check_name, "TR04: declaration of '%0' not within package namespace '%1'", true)
                    << decl->getSourceRange()
                    << named->getNameAsString()
                    << (analyser.config()->toplevel_namespace() + "::" + analyser.package())
                    ;
            }
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &::check);
