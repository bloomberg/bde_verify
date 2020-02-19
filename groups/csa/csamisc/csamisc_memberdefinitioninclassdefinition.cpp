// csamisc_memberdefinitioninclassdefinition.cpp                      -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Casting.h>
#include <map>
#include <string>

using namespace clang;
using namespace csabase;

// -----------------------------------------------------------------------------

static std::string const check_name("member-definition-in-class-definition");

// -----------------------------------------------------------------------------

namespace
{
    struct member_definition
    {
        std::map<void const*, bool> reported_;
    };
}

// -----------------------------------------------------------------------------

static void
member_definition_in_class_definition(Analyser& analyser,
                                      CXXMethodDecl const* decl)
{
    member_definition& data = analyser.attachment<member_definition>();

    if (decl->isTemplateInstantiation()) {
        if (CXXMethodDecl const* tplt = llvm::dyn_cast<CXXMethodDecl>(
                decl->getTemplateInstantiationPattern())) {
            decl = tplt;
        }
    }

    if (decl->getLexicalDeclContext() == decl->getDeclContext()
        && decl->hasInlineBody()
        && !decl->getParent()->isLocalClass()
        && !decl->isImplicit()
        && !data.reported_[decl->getCanonicalDecl()]
        && !analyser.is_test_driver()
        && !decl->getBeginLoc().isMacroID())
    {
        analyser.report(decl, check_name, "CD01",
                "Member function '%0' is defined in the class definition.")
            << decl->getQualifiedNameAsString();
        data.reported_[decl->getCanonicalDecl()] = true;
    }
}

// -----------------------------------------------------------------------------

static RegisterCheck check(check_name, &member_definition_in_class_definition);

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
