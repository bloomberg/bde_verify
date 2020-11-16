// csamisc_charvsstring.cpp                                           -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/CanonicalType.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/OperationKinds.h>
#include <clang/AST/Type.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <string>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("char-vs-string");

// -----------------------------------------------------------------------------

static void check(Analyser& analyser,
                  const Expr *expr,
                  const Expr * const *args,
                  unsigned numArgs,
                  const FunctionDecl* decl)
{
    QualType charConst(analyser.context()->CharTy.withConst());
    for (unsigned index(0); index != numArgs; ++index) {
        QualType canonArg(args[index]->getType().getCanonicalType());
        bool isCharPointer(
            canonArg.getTypePtr()->isPointerType() &&
            canonArg.getTypePtr()->getPointeeType().getCanonicalType() ==
                charConst);
        Expr const* arg(isCharPointer? args[index]: 0);
        arg = arg? arg->IgnoreParenCasts(): 0;
        UnaryOperator const* unary(arg? llvm::dyn_cast<UnaryOperator>(arg): 0);
        if (unary && unary->getOpcode() == UO_AddrOf) {
            Expr const* sub(unary->getSubExpr()->IgnoreParenCasts());
            DeclRefExpr const* ref(llvm::dyn_cast<DeclRefExpr>(sub));
            if (ref && ref->getType().getCanonicalType() ==
                           analyser.context()->CharTy) {
                auto report = analyser.report(args[index], check_name, "ADC01",
                                "Passing address of char '%0' where a "
                                "null-terminated string may be expected");
                report << ref->getDecl()->getName();
            }
        }
    }
}

// -----------------------------------------------------------------------------

static void checkCall(Analyser& analyser, CallExpr const* expr)
{
    if (FunctionDecl const* decl = expr->getDirectCallee()) {
        check(analyser,
              expr,
              const_cast<CallExpr*>(expr)->getArgs(),
              expr->getNumArgs(),
              decl);
    }
}

static void checkCtor(Analyser& analyser, CXXConstructExpr const* expr)
{
    check(analyser,
          expr,
          expr->getArgs(),
          expr->getNumArgs(),
          expr->getConstructor());
}

// -----------------------------------------------------------------------------

static RegisterCheck register_check0(check_name, &checkCall);
static RegisterCheck register_check1(check_name, &checkCtor);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
