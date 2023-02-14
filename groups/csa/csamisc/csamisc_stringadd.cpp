// csamisc_stringadd.cpp                                              -*-C++-*-

#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/Casting.h>
#include <clang/AST/Expr.h>
#include <clang/AST/OperationKinds.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("string-add");

static bool is_addition(Analyser& analyser,
                        Expr const* str,
                        Expr const* value,
                        BinaryOperatorKind op)
{
    if (StringLiteral const* lit =
            llvm::dyn_cast<StringLiteral>(str->IgnoreParenCasts())) {
        llvm::APSInt length(32, false);
        llvm::APSInt zero(32, false);
        length = lit->getByteLength();
        zero = 0u;
        value = value->IgnoreParenCasts();
        llvm::Optional<llvm::APSInt> result =
                            value->getIntegerConstantExpr(*analyser.context());
        return !result ||
               (op == BO_Add && (*result < zero || length < *result)) ||
               (op == BO_Sub && (zero < *result || length + *result < zero));
    }
    return false;
}

static void check(Analyser& analyser, BinaryOperator const* expr)
{
    if ((expr->getOpcode() == BO_Add || expr->getOpcode() == BO_Sub) &&
        !expr->getType()->isDependentType() &&
        (is_addition(
             analyser, expr->getLHS(), expr->getRHS(), expr->getOpcode()) ||
         is_addition(
             analyser, expr->getRHS(), expr->getLHS(), expr->getOpcode()))) {
        analyser.report(expr->getOperatorLoc(), check_name, "SA01",
                        "%0 integer %1 string literal")
            << expr->getSourceRange()
            << (expr->getOpcode() == BO_Add? "Adding": "Subtracting")
            << (expr->getOpcode() == BO_Add? "to": "from");
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
