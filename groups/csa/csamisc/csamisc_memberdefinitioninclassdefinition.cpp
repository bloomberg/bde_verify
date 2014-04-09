// csamisc_memberdefinitioninclassdefinition.cpp                      -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <map>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("member-definition-in-class-definition");

// -----------------------------------------------------------------------------

namespace
{
    struct member_definition
    {
        std::map<void const*, bool> reported_;
    };
}

// -----------------------------------------------------------------------------

static void
member_definition_in_class_definition(bde_verify::csabase::Analyser& analyser,
                                      clang::CXXMethodDecl const* decl)
{
    member_definition& data = analyser.attachment<member_definition>();

    clang::CXXConstructorDecl const* ctor =
        llvm::dyn_cast<clang::CXXConstructorDecl>(decl);
    clang::CXXDestructorDecl const* dtor =
        llvm::dyn_cast<clang::CXXDestructorDecl>(decl);

    if (decl->isTemplateInstantiation()) {
        if (clang::CXXMethodDecl const* tplt =
                llvm::dyn_cast<clang::CXXMethodDecl>(
                    decl->getTemplateInstantiationPattern())) {
            decl = tplt;
        }
    }

    if (decl->getLexicalDeclContext() == decl->getDeclContext()
        && decl->hasInlineBody()
        && !decl->isTrivial()
        && !decl->getParent()->isLocalClass()
        && (!ctor || !ctor->isImplicit())
        && (!dtor || !dtor->isImplicit())
        && !data.reported_[decl->getCanonicalDecl()]
        && !analyser.is_test_driver()
        && !decl->getLocStart().isMacroID())
    {
        analyser.report(decl, check_name, "CD01",
                "Member function '%0' is defined in the class definition.")
            << decl->getQualifiedNameAsString();
        data.reported_[decl->getCanonicalDecl()] = true;
    }
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck check(check_name, &member_definition_in_class_definition);
