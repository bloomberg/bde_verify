// groups/csa/csatr/csatr_nesteddeclarations.cpp                      -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_format.h>
#include <csabase_cast_ptr.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("nested-declarations");

// ----------------------------------------------------------------------------

static bool
is_swap(clang::FunctionDecl const* decl)
{
    return decl->getNameAsString() == "swap"
        && decl->getNumParams() == 2
        && decl->getParamDecl(0)->getType() == decl->getParamDecl(1)->getType()
        && decl->getParamDecl(0)->getType().getTypePtr()->isReferenceType()
        && !decl->getParamDecl(0)->getType().getNonReferenceType().isConstQualified();
}

// ----------------------------------------------------------------------------

static bool
isSpecialFunction(clang::NamedDecl const* decl)
{
    cool::csabase::cast_ptr<clang::FunctionDecl> function(decl);
    return function
        && (function->isOverloadedOperator() || is_swap(function.get()));
}

// ----------------------------------------------------------------------------

static void
check(cool::csabase::Analyser& analyser, clang::Decl const* decl)
{
    if (analyser.is_main() &&
        analyser.config()->value("main_namespace_check") == "off") {
        return;                                                       // RETURN
    }

    if (analyser.is_global_package()) {
        return;                                                       // RETURN
    }

    cool::csabase::Location location(analyser.get_location(decl));
    clang::NamedDecl const* named(llvm::dyn_cast<clang::NamedDecl>(decl));
    if (analyser.is_component(location.file()) && named) {
        if (llvm::dyn_cast<clang::NamespaceDecl>(decl)
            || llvm::dyn_cast<clang::UsingDirectiveDecl>(decl)) {
            // namespace declarations are permissible e.g. for forward declarations.
            return;                                                   // RETURN
        }
        else if (clang::TagDecl const* tag = llvm::dyn_cast<clang::TagDecl>(decl)) {
            // Forward declarations are always permissible but definitions are not.
            if (!tag->isThisDeclarationADefinition()
                && analyser.is_component_header(location.file())) {
                return;                                               // RETURN
            }
        }
        else if (llvm::dyn_cast<clang::NamespaceAliasDecl>(decl)
                 && analyser.is_component_source(decl)) {
            return;                                                   // RETURN
        }

        clang::DeclContext const* context(decl->getDeclContext());
        std::string name = named->getNameAsString();
        if (llvm::dyn_cast<clang::TranslationUnitDecl>(context)) {
            if (name != "main"
                && name != "RCSId"
                && !llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)
                && !llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl)
                && name.find("operator new") == std::string::npos
                && name.find("operator delete") == std::string::npos
                ) {
                analyser.report(decl, check_name, "TR04: "
                                "declaration of '%0' at global scope", true)
                    << decl->getSourceRange()
                    << name;
            }
            return;                                                   // RETURN
        }

        clang::NamespaceDecl const* space =
            llvm::dyn_cast<clang::NamespaceDecl>(context);
        if (!space) {
            return;                                                   // RETURN
        }

        std::string pkgns = analyser.package();
        if (   space->isAnonymousNamespace()
            && llvm::dyn_cast<clang::NamespaceDecl>(space->getDeclContext())) {
            space =
                llvm::dyn_cast<clang::NamespaceDecl>(space->getDeclContext());
        }
        if (space->getNameAsString() ==
            analyser.config()->toplevel_namespace()) {
            // No package namespace.  This is OK if no package namespace has
            // been seen, for the sake of legacy "package_name" components.
            clang::DeclContext::decl_iterator b = space->decls_begin();
            clang::DeclContext::decl_iterator e = space->decls_end();
            bool found = false;
            while (!found && b != e) {
                const clang::NamespaceDecl *ns =
                    llvm::dyn_cast<clang::NamespaceDecl>(*b++);
                found = ns && ns->getNameAsString() == analyser.package();
            }
            if (!found) {
                pkgns = analyser.config()->toplevel_namespace();
            }
        }
        if (name.length() > 0)
        {
            std::string spname = space->getNameAsString();
            clang::NamespaceDecl const* outer = space;
            if (pkgns == analyser.package()) {
                outer = llvm::dyn_cast<clang::NamespaceDecl>(
                    space->getDeclContext());
            }
            if ((spname != pkgns
                 || !outer
                 || outer->getNameAsString() !=
                    analyser.config()->toplevel_namespace())
                && (   spname != analyser.config()->toplevel_namespace()
                    || name.find(analyser.package() + '_') != 0
                    )
                && !llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)
                && !llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl)
                && name.find("operator new") == std::string::npos
                && name.find("operator delete") == std::string::npos
                && !isSpecialFunction(named)
                && (   analyser.is_component_header(named)
                    || named->hasLinkage()
                    )
                )
            {
                //-dk:TODO check if this happens in the correct namespace
                analyser.report(decl, check_name, "TR04: declaration of '%0' not within package namespace '%1'", true)
                    << decl->getSourceRange()
                    << name
                    << (analyser.config()->toplevel_namespace() + "::" + analyser.package())
                    ;
            }
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &check);
