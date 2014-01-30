// csamisc_cstylecastused.cpp                                         -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#ident "$Id$"

using cool::csabase::Analyser;
using clang::CStyleCastExpr;
using clang::FunctionDecl;
using clang::FunctionTemplateDecl;
using clang::Stmt;

// ----------------------------------------------------------------------------

static std::string const check_name("c-cast");

static void
check_cast(Analyser& analyser, CStyleCastExpr const* expr)
{
    if (expr->getCastKind() != clang::CK_ToVoid &&
        !expr->getLocStart().isMacroID()) {
        analyser.report(expr, check_name, "CC01", "C-style cast is used")
            << expr->getSourceRange();
    }
}

static void find_casts(Analyser& analyser, const Stmt *stmt)
{
    Stmt::const_child_iterator b = stmt->child_begin();
    Stmt::const_child_iterator e = stmt->child_end();
    for (Stmt::const_child_iterator i = b; i != e; ++i) {
        if (*i) {
            find_casts(analyser, *i);
            const CStyleCastExpr *expr = llvm::dyn_cast<CStyleCastExpr>(*i);
            if (expr) {
                check_cast(analyser, expr);
            }
        }
    }
}

static void check_f(Analyser& analyser, FunctionDecl const* decl)
{
    if (decl->hasBody() &&
        decl->getBody() &&
        !decl->isTemplateInstantiation()) {
        find_casts(analyser, decl->getBody());
    }
}

static void check_ft(Analyser& analyser, FunctionTemplateDecl const* decl)
{
    check_f(analyser, decl->getTemplatedDecl());
}

// ----------------------------------------------------------------------------

static cool::csabase::RegisterCheck c1(check_name, &check_f);
static cool::csabase::RegisterCheck c2(check_name, &check_ft);
