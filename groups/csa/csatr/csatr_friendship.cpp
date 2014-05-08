// csatr_friendship.cpp                                               -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <csabase_format.h>
#include <clang/AST/DeclFriend.h>
#ident "$Id$"

using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("local-friendship-only");

// ----------------------------------------------------------------------------

static bool
is_extern_type(csabase::Analyser&  analyser,
              Type const         *type)
{
    CXXRecordDecl const* record(type->getAsCXXRecordDecl());
    csabase::Location cloc(analyser.get_location(record));
    return (type->isIncompleteType()
            && record->getLexicalDeclContext()->isFileContext())
        || !analyser.is_component(cloc.file());
}

// ----------------------------------------------------------------------------

static void
local_friendship_only(csabase::Analyser&  analyser,
                      FriendDecl const  *decl)
{
    const NamedDecl *named = decl->getFriendDecl();
    const TypeSourceInfo *tsi = decl->getFriendType();
    const Type *type = tsi ? tsi->getTypeLoc().getTypePtr() : 0;
    if (type && type->isElaboratedTypeSpecifier()) {
        type = type->getAs<ElaboratedType>()->getNamedType().getTypePtr();
        if (const TemplateSpecializationType* spec =
                type->getAs<TemplateSpecializationType>()) {
            named = spec->getTemplateName().getAsTemplateDecl();
        }
    }

    if (named) {
        if (CXXMethodDecl const* method
            = llvm::dyn_cast<CXXMethodDecl>(named)) {
            if (!analyser.is_component(method->getParent())) {
                analyser.report(decl->getFriendLoc(), check_name, "TR19",
                                "Friendship to a method "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else if (FunctionDecl const* function
                 = llvm::dyn_cast<FunctionDecl>(named)) {
            if (!analyser.is_component(function->getCanonicalDecl())) {
                analyser.report(decl->getFriendLoc(), check_name, "TR19",
                                "Friendship to a function "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else if (FunctionTemplateDecl const* function
                 = llvm::dyn_cast<FunctionTemplateDecl>(named)) {
            if (!analyser.is_component(function->getCanonicalDecl())) {
                analyser.report(decl->getFriendLoc(), check_name,  "TR19",
                                "Friendship to a function template "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else if (ClassTemplateDecl const* cls
                 = llvm::dyn_cast<ClassTemplateDecl>(named)) {
            if (!analyser.is_component(cls->getCanonicalDecl())) {
                analyser.report(decl->getFriendLoc(), check_name, "TR19",
                                "Friendship to a class template "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else {
            analyser.report(decl, check_name,  "TR19",
                            "Unknonwn kind of friendship (%0)")
                << decl->getSourceRange()
                << csabase::format(named->getKind());
        }
    }
    else {
        TypeSourceInfo const* typeInfo(decl->getFriendType());
        TypeLoc loc(typeInfo->getTypeLoc());
        Type const* type(loc.getTypePtr());
        if (is_extern_type(analyser, type)) {
            analyser.report(decl, check_name, "TR19",
                            "Friendship to a class "
                            "can only be granted within a component"
                            )
                << decl->getSourceRange();
        }
    }
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck check(check_name,
                                          &local_friendship_only);
