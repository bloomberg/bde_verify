// csamisc_boolcomparison.cpp                                         -*-C++-*-

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/OperationKinds.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Casting.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("boolcomparison");

// ----------------------------------------------------------------------------

static bool
is_bool_comparison(Expr* expr0, Expr* expr1)
{
    expr0 = expr0->IgnoreParenCasts();
    if (llvm::dyn_cast<CXXBoolLiteralExpr>(expr0))
    {
        expr1 = expr1->IgnoreParenCasts();
        return expr0->getType().getUnqualifiedType().getCanonicalType()
            == expr1->getType().getUnqualifiedType().getCanonicalType();
    }
    return false;
}

// ----------------------------------------------------------------------------

static bool
is_comparison(BinaryOperatorKind opcode)
{
    return opcode == BO_LT
        || opcode == BO_GT
        || opcode == BO_LE
        || opcode == BO_GE
        || opcode == BO_EQ
        || opcode == BO_NE
        ;
}

// ----------------------------------------------------------------------------

static void
check(Analyser& analyser, BinaryOperator const* expr)
{
    if (is_comparison(expr->getOpcode())
        && (is_bool_comparison(expr->getLHS(), expr->getRHS())
            || is_bool_comparison(expr->getRHS(), expr->getLHS())))
    {
        analyser.report(expr->getLHS(), check_name, "BC01",
                        "Comparing a Boolean expression to a Boolean literal")
            << expr->getSourceRange();
    }
}

// ----------------------------------------------------------------------------

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
