// csamisc_constantreturn.cpp                                         -*-C++-*-

#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/Casting.h>

#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Redeclarable.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>

#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>

#include <iterator>
#include <string>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("constant-return");

// -----------------------------------------------------------------------------

static void check(Analyser& analyser, FunctionDecl const* decl)
{
    if (analyser.is_component(decl)
        && decl->hasBody()
        && decl->getBody()
        && decl->getIdentifier())
    {
        Stmt* stmt(decl->getBody());
        while (llvm::dyn_cast<CompoundStmt>(stmt) &&
               std::distance(stmt->child_begin(), stmt->child_end()) == 1) {
            stmt = *stmt->child_begin();
        }

        if (llvm::dyn_cast<ReturnStmt>(stmt)
            && llvm::dyn_cast<ReturnStmt>(stmt)->getRetValue())
        {
            ReturnStmt* ret(llvm::dyn_cast<ReturnStmt>(stmt));
            Expr* expr(ret->getRetValue());
            llvm::Optional<llvm::APSInt> result;
            if (!expr->isValueDependent() &&
                (result = expr->getIntegerConstantExpr(*analyser.context())))
            {
                analyser.report(expr, check_name, "CR01",
                                "Function '%0' has only one statement which "
                                "returns the constant '%1'")
                    << decl->getNameAsString()
                    << toString(*result, 10)
                    << decl->getNameInfo().getSourceRange();
                for (FunctionDecl::redecl_iterator it(decl->redecls_begin()),
                     end(decl->redecls_end());
                     it != end;
                     ++it) {
                    analyser.report(*it, check_name, "CR01",
                                    "Declaration of '%0' (which always "
                                    "returns the constant %1)", false,
                                    DiagnosticIDs::Note)
                        << decl->getNameAsString()
                        << toString(*result, 10)
                        << decl->getNameInfo().getSourceRange();
                }
            }
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
