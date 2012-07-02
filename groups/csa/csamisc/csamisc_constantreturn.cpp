// csamisc_constantreturn.t.cpp                                       -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------


#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_abstractvisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <algorithm>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("constant-return");

// -----------------------------------------------------------------------------

static void
check(cool::csabase::Analyser& analyser, clang::FunctionDecl const* decl)
{
    if (decl->hasBody() && decl->getIdentifier())
    {
        clang::Stmt* stmt(decl->getBody());
        while (llvm::dyn_cast<clang::CompoundStmt>(stmt) && std::distance(stmt->child_begin(), stmt->child_end()) == 1)
        {
            stmt = *stmt->child_begin();
        }

        if (llvm::dyn_cast<clang::ReturnStmt>(stmt)
            && llvm::dyn_cast<clang::ReturnStmt>(stmt)->getRetValue())
        {
            clang::ReturnStmt* ret(llvm::dyn_cast<clang::ReturnStmt>(stmt));
            clang::Expr* expr(ret->getRetValue());
            llvm::APSInt result;
            if (expr->isIntegerConstantExpr(result, *analyser.context()))
            {
                analyser.report(expr, check_name, "function %0() has only one statement which returns the constant '%1'") 
                    << decl->getNameAsString()
                    << result.toString(10);
                for (clang::FunctionDecl::redecl_iterator it(decl->redecls_begin()), end(decl->redecls_end()); it != end; ++it)
                {
                    analyser.report(*it, check_name, "declaration of %0() (which always returns the constant %1)")
                        << decl->getNameAsString()
                        << result.toString(10);
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &::check);
