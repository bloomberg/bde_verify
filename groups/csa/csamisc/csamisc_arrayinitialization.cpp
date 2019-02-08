// csamisc_arrayinitialization.cpp                                    -*-C++-*-

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/APInt.h>
#include <llvm/Support/Casting.h>
#include <set>
#include <string>
#include <utility>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("array-initialization");

// -----------------------------------------------------------------------------

static bool isDefaultConstructor(Analyser& analyser, Expr const* init)
{
    CXXConstructExpr const* ctor = llvm::dyn_cast<CXXConstructExpr>(init);
    return ctor && (ctor->getNumArgs() == 0 ||
                    (ctor->getNumArgs() == 1 &&
                     llvm::dyn_cast<CXXDefaultArgExpr>(ctor->getArg(0))));
}

// -----------------------------------------------------------------------------

static bool
isDefaultValue(Analyser& analyser, InitListExpr const* expr, Expr const* init)
{
    Expr const* orig(init);
    do
    {
        orig = init;
        init = const_cast<Expr*>(init)->IgnoreImplicit();

        if (CastExpr const* cast = llvm::dyn_cast<CastExpr>(init))
        {
            init = cast->getSubExpr();
        } else if (CXXConstructExpr const* ctor =
                       llvm::dyn_cast<CXXConstructExpr>(init)) {
            if (ctor->getNumArgs() == 1
                && llvm::dyn_cast<MaterializeTemporaryExpr>(ctor->getArg(0)))
            {
                init = llvm::dyn_cast<MaterializeTemporaryExpr>(
                    ctor->getArg(0))->GetTemporaryExpr();
            }
        }
    }
    while (orig != init);

    return llvm::dyn_cast<CXXScalarValueInitExpr>(init) ||
           (llvm::dyn_cast<CharacterLiteral>(init) &&
            llvm::dyn_cast<CharacterLiteral>(init)->getValue() == 0) ||
           (llvm::dyn_cast<IntegerLiteral>(init) &&
            llvm::dyn_cast<IntegerLiteral>(init)
                    ->getValue()
                    .getLimitedValue() == 0u) ||
           isDefaultConstructor(analyser, init);
}

// -----------------------------------------------------------------------------

namespace
{
    struct reported
    {
        std::set<void const*> reported_;
    };
}

static void check(Analyser& analyser, InitListExpr const* expr)
{
    Type const* type(expr->getType().getTypePtr());
    if (type->isConstantArrayType()
        && !expr->isStringLiteralInit()
        )
    {
        ConstantArrayType const* array(
            analyser.context()->getAsConstantArrayType(expr->getType()));
        if (0u < expr->getNumInits() &&
            expr->getNumInits() < array->getSize().getLimitedValue() &&
            !isDefaultValue(
                 analyser, expr, expr->getInit(expr->getNumInits() - 1u)) &&
            analyser.attachment<reported>().reported_.insert(expr).second) {
            analyser.report(expr, check_name, "II01",
                    "Incomplete initialization with non-defaulted last value")
                << expr->getInit(expr->getNumInits() - 1u)->getSourceRange();
        }
    }
}

// -----------------------------------------------------------------------------

static RegisterCheck register_check(check_name, &check);

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
