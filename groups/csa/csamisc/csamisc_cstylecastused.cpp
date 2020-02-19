// csamisc_cstylecastused.cpp                                         -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/OperationKinds.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Casting.h>
#include <string>

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("c-cast");

// ----------------------------------------------------------------------------

static void
check_cast(Analyser& analyser, CStyleCastExpr const* expr)
{
    switch (expr->getCastKind()) {
      case CK_NullToPointer:
      case CK_NullToMemberPointer:
      case CK_MemberPointerToBoolean:
      case CK_PointerToBoolean:
      case CK_ToVoid:
      case CK_IntegralCast:
      case CK_IntegralToBoolean:
      case CK_IntegralToFloating:
      case CK_FloatingToIntegral:
      case CK_FloatingToBoolean:
      case CK_FloatingCast:
        break;
      default: {
          if (!expr->getBeginLoc().isMacroID() &&
              !expr->getSubExprAsWritten()->isNullPointerConstant(
                   *analyser.context(), Expr::NPC_ValueDependentIsNotNull)) {
            analyser.report(expr, check_name, "CC01", "C-style cast is used")
                << expr->getSourceRange();
        }
      } break;
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

static RegisterCheck c1(check_name, &check_f);
static RegisterCheck c2(check_name, &check_ft);

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
