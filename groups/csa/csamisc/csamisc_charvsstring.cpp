// csamisc_charvsstring.cpp                                           -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <csabase_ppobserver.h>
#include <fstream>
#include <string>

static std::string const check_name("char-vs-string");

// -----------------------------------------------------------------------------

static void
check(cool::csabase::Analyser&    analyser,
      const clang::Expr          *expr,
      clang::Expr               **args,
      unsigned                    numArgs,
      const clang::FunctionDecl  *decl)
{
    clang::QualType charConst(analyser.context()->CharTy.withConst());
    for (unsigned index(0); index != numArgs; ++index) {
        clang::QualType canonArg(args[index]->getType().getCanonicalType());
        bool isCharPointer(canonArg.getTypePtr()->isPointerType()
                           && canonArg.getTypePtr()->getPointeeType().getCanonicalType() == charConst);
        clang::Expr const* arg(isCharPointer? args[index]: 0);
        arg = arg? arg->IgnoreParenCasts(): 0;
        clang::UnaryOperator const* unary(arg? llvm::dyn_cast<clang::UnaryOperator>(arg): 0);
        if (unary && unary->getOpcode() == clang::UO_AddrOf) {
            clang::Expr const* sub(unary->getSubExpr()->IgnoreParenCasts());
            clang::DeclRefExpr const* ref(llvm::dyn_cast<clang::DeclRefExpr>(sub));
            if (ref && ref->getType().getCanonicalType() == analyser.context()->CharTy) {
                analyser.report(args[index], check_name,
                                "passing address of char '%0' where probably a null-terminated string is expected")
                    << ref->getDecl()->getName();
                    ;
            }
        }
    }
}

// -----------------------------------------------------------------------------

static void
checkCall(cool::csabase::Analyser& analyser, clang::CallExpr const* expr)
{
    if (clang::FunctionDecl const* decl = expr->getDirectCallee()) {
        ::check(analyser, expr, const_cast<clang::CallExpr*>(expr)->getArgs(), expr->getNumArgs(), decl);
    }
}

static void
checkCtor(cool::csabase::Analyser& analyser, clang::CXXConstructExpr const* expr)
{
    ::check(analyser, expr, expr->getArgs(), expr->getNumArgs(), expr->getConstructor());
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check0(check_name, &::checkCall);
static cool::csabase::RegisterCheck register_check1(check_name, &::checkCtor);
