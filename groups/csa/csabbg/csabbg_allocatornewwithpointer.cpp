// csabbg_allocatornewwithpointer.cpp                                 -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("allocator-new");

static void check(Analyser& analyser, CXXNewExpr const* expr)
{
    if (expr->getNumPlacementArgs() == 1
        && expr->getPlacementArg(0)->getType()->isPointerType())
    {
        Expr* placement(const_cast<Expr*>(expr->getPlacementArg(0))
                            ->IgnoreParenImpCasts());
        if (placement && placement->getType()->isPointerType()) {
            QualType pointee(placement->getType()->getPointeeType());
            TypeDecl* bslma_allocator(
                analyser.lookup_type("::BloombergLP::bslma_Allocator"));

            if (bslma_allocator && bslma_allocator->getTypeForDecl() &&
                bslma_allocator->getTypeForDecl()
                        ->getCanonicalTypeInternal() ==
                    pointee->getCanonicalTypeInternal()) {
                auto report = analyser.report(placement, check_name, "ANP01",
                        "Allocator new with pointer");
                report << placement->getSourceRange();
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
