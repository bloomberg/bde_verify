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

using namespace clang;
using namespace csabase;

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
member_definition_in_class_definition(Analyser& analyser,
                                      CXXMethodDecl const* decl)
{
    member_definition& data = analyser.attachment<member_definition>();

    if (decl->isTemplateInstantiation()) {
        if (CXXMethodDecl const* tplt = llvm::dyn_cast<CXXMethodDecl>(
                decl->getTemplateInstantiationPattern())) {
            decl = tplt;
        }
    }

    if (decl->getLexicalDeclContext() == decl->getDeclContext()
        && decl->hasInlineBody()
        && !decl->getParent()->isLocalClass()
        && !decl->isImplicit()
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

static RegisterCheck check(check_name, &member_definition_in_class_definition);
