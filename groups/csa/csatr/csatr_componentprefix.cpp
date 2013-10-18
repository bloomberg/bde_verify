// csatr_componentprefix.cpp                                          -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <csabase_format.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <sstream>
#include <cctype>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("component-prefix");

// ----------------------------------------------------------------------------

static char
to_lower(unsigned char c)
{
    return std::tolower(c);
}

static bool
wrong_prefix(cool::csabase::Analyser& analyser,
             std::string              name)
{
    std::transform(name.begin(), name.end(), name.begin(), &to_lower);
    return (name.size() < analyser.component().size()
            || name.substr(0, analyser.component().size()) != analyser.component())
        && (false //-dk:TODO !analyser.package_legacy()
            || name.find(analyser.package() + '_' + analyser.component()))
        ;
}

// ----------------------------------------------------------------------------

static void
component_prefix(cool::csabase::Analyser&  analyser,
                 clang::Decl const        *decl)
{
    clang::NamedDecl const* named(llvm::dyn_cast<clang::NamedDecl>(decl));
    std::string const& name(named? named->getNameAsString(): std::string());
    if (!name.empty()
        && !named->isCXXClassMember()
        && (named->hasLinkage()
            || llvm::dyn_cast<clang::TypedefDecl>(named))
        && !llvm::dyn_cast<clang::NamespaceDecl>(named)
        && !llvm::dyn_cast<clang::UsingDirectiveDecl>(named)
        && !llvm::dyn_cast<clang::UsingDecl>(named)
        && wrong_prefix(analyser, name)
        && analyser.is_component_header(decl)
        && (!llvm::dyn_cast<clang::RecordDecl>(named)
            || llvm::dyn_cast<clang::RecordDecl>(named)->isCompleteDefinition()
            )
        && (!llvm::dyn_cast<clang::FunctionDecl>(named)
            || !(llvm::dyn_cast<clang::FunctionDecl>(named)->isOverloadedOperator()
                 || named->getNameAsString() == "swap")
            )
        && (!llvm::dyn_cast<clang::FunctionTemplateDecl>(named)
            || !(llvm::dyn_cast<clang::FunctionTemplateDecl>(named)->getTemplatedDecl()->isOverloadedOperator()
                 || named->getNameAsString() == "swap")
            )
        && !llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)
        && !llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl)
        && !llvm::dyn_cast<clang::FunctionDecl>(decl->getDeclContext())
        && !(llvm::dyn_cast<clang::CXXRecordDecl>(decl)
             && llvm::dyn_cast<clang::CXXRecordDecl>(decl)->getDescribedClassTemplate())
        ) {
        analyser.report(decl, ::check_name,
                        "TR05: globally visible identifier '%0' "
                        "without component prefix")
            << named->getQualifiedNameAsString()
            ;
    }
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck check(check_name, &::component_prefix);
