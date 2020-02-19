// csamisc_swapab.cpp                                                 -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/StringRef.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("swap-a-b");

// ----------------------------------------------------------------------------

namespace
{

void allFunDecls(Analyser& analyser, const FunctionDecl* func)
    // Callback function for inspecting function declarations.
{
    const ParmVarDecl *pa;
    const ParmVarDecl *pb;

    if (   func->getDeclName().isIdentifier()
        && func->getName() == "swap"
        && func->getNumParams() == 2
        && (pa = func->getParamDecl(0))->getType() ==
           (pb = func->getParamDecl(1))->getType()
        && (   pa->getType()->isPointerType()
            || pa->getType()->isReferenceType()
            )
        ) {
        if (!pa->getName().empty() && pa->getName() != "a") {
            analyser.report(pa->getBeginLoc(), check_name, "SWAB01",
                            "First parameter of 'swap' should be named 'a'")
                << pa->getSourceRange();
        }
        if (!pb->getName().empty() && pb->getName() != "b") {
            analyser.report(pb->getBeginLoc(), check_name, "SWAB01",
                            "Second parameter of 'swap' should be named 'b'")
                << pb->getSourceRange();
        }
    }
}

void allTpltFunDecls(Analyser& analyser, const FunctionTemplateDecl* func)
    // Callback function for inspecting function template declarations.
{
    allFunDecls(analyser, func->getTemplatedDecl());
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &allFunDecls);
static RegisterCheck c2(check_name, &allTpltFunDecls);

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
