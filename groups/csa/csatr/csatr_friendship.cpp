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

// ----------------------------------------------------------------------------

static std::string const check_name("local-friendship-only");

// ----------------------------------------------------------------------------

static bool
is_extern_type(cool::csabase::Analyser&  analyser,
              clang::Type const         *type)
{
    clang::CXXRecordDecl const* record(type->getAsCXXRecordDecl());
    cool::csabase::Location cloc(analyser.get_location(record));
    return (type->isIncompleteType()
            && record->getLexicalDeclContext()->isFileContext())
        || !analyser.is_component(cloc.file());
}

// ----------------------------------------------------------------------------

static void
local_friendship_only(cool::csabase::Analyser&  analyser,
                      clang::FriendDecl const  *decl)
{
    if (clang::NamedDecl const* named = decl->getFriendDecl()) {
        if (clang::CXXMethodDecl const* method
            = llvm::dyn_cast<clang::CXXMethodDecl>(named)) {
            if (!analyser.is_component(method->getParent())) {
                analyser.report(decl->getFriendLoc(), check_name, "TR19",
                                "Friendship to a method "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else if (clang::FunctionDecl const* function
                 = llvm::dyn_cast<clang::FunctionDecl>(named)) {
            if (!analyser.is_component(function->getCanonicalDecl())) {
                analyser.report(decl->getFriendLoc(), check_name, "TR19",
                                "Friendship to a function "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else if (clang::FunctionTemplateDecl const* function
                 = llvm::dyn_cast<clang::FunctionTemplateDecl>(named)) {
            if (!analyser.is_component(function->getCanonicalDecl())) {
                analyser.report(decl->getFriendLoc(), check_name,  "TR19",
                                "Friendship to a function template "
                                "can only be granted within a component")
                    << decl->getSourceRange();
            }
        }
        else if (clang::ClassTemplateDecl const* cls
                 = llvm::dyn_cast<clang::ClassTemplateDecl>(named)) {
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
                << cool::csabase::format(named->getKind());
        }
    }
    else {
        clang::TypeSourceInfo const* typeInfo(decl->getFriendType());
        clang::TypeLoc loc(typeInfo->getTypeLoc());
        clang::Type const* type(loc.getTypePtr());
        if (is_extern_type(analyser, type)) {
            analyser.report(decl, check_name, "TR19",
                            "Friendship to a class "
                            "can only be granted within a component"
                            )
                << decl->getSourceRange()
                ;
        }
    }
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name,
                                          &local_friendship_only);
