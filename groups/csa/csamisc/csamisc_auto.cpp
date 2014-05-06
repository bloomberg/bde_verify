// csamisc_auto.cpp                                         -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("auto");

static void
check(csabase::Analyser& analyser, clang::VarDecl const* decl)
{
    if (decl->hasDefinition() == clang::VarDecl::Definition) {
        clang::TypeSourceInfo const* tsinfo(decl->getTypeSourceInfo());
        clang::Type const* type(tsinfo->getTypeLoc().getTypePtr());
        if (clang::ReferenceType const* ref
            = llvm::dyn_cast<clang::ReferenceType>(type)) {
            type = ref->getPointeeType().getTypePtr();
        }
        if (clang::AutoType const* at = llvm::dyn_cast<clang::AutoType>(type)){
            clang::Expr const* expr = decl->getInit();
            expr = expr? expr->IgnoreParenCasts(): expr;
            std::string exprType(expr? expr->getType().getAsString(): "<none>");
            clang::QualType deduced(at->getDeducedType());
            analyser.report(decl, check_name, "AU01", "VarDecl: %0 %1 %2")
                << (expr? expr->getSourceRange(): decl->getSourceRange())
                << deduced.getAsString()
                << tsinfo->getType().getAsString()
                << exprType
                ;
        }
    }
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck register_check(check_name, &check);
