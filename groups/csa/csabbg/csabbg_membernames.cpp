// csabbg_membernames.cpp                                             -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("member-names");

// ----------------------------------------------------------------------------

static void check_private(Analyser& a, DeclaratorDecl const *decl)
{
    auto rd = llvm::dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (decl->getAccess() != AS_private &&
        rd &&
        !rd->getTemplateInstantiationPattern() &&
        rd->getTagKind() == TTK_Class) {
        a.report(decl, check_name, "MN01",
                 "Class data members must be private");
    }
}

static void check_pointer(Analyser& a, DeclaratorDecl const *decl)
{
    if (decl->getType()->getTypeClass() == Type::Typedef) {
        return;
    }
    auto rd = llvm::dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (rd && rd->getTemplateInstantiationPattern()) {
        return;                                                       // RETURN
    }
    bool is_pointer_type = decl->getType()->isPointerType();
    bool is_pointer_name = decl->getName().endswith("_p");
    if (is_pointer_type && !is_pointer_name) {
        a.report(decl, check_name, "MN04",
                 "Pointer member names must end in '_p'");
    }
    else if (!is_pointer_type && is_pointer_name) {
        a.report(decl, check_name, "MN05",
                 "Only pointer member names should end in '_p'");
    }
}

static void field_name(Analyser& a, FieldDecl const *decl)
{
    auto rd = llvm::dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (rd && rd->getTemplateInstantiationPattern()) {
        return;                                                       // RETURN
    }
    if (decl->isCXXClassMember()) {
        check_private(a, decl);
        if (!decl->getName().startswith("d_")) {
            a.report(decl, check_name, "MN02",
                     "Non-static data member names must begin with 'd_'");
        }
        check_pointer(a, decl);
    }
}

static void var_name(Analyser& a, VarDecl const *decl)
{
    auto rd = llvm::dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (rd && rd->getTemplateInstantiationPattern()) {
        return;                                                       // RETURN
    }
    if (decl->isCXXClassMember()) {
        if (!decl->getType().isConstQualified()) {
            check_private(a, decl);
            if (!decl->getName().startswith("s_")) {
                a.report(decl, check_name, "MN03",
                         "Static data member names must begin with 's_'");
            }
        }
        else {
            if (!decl->getName().startswith("s_") &&
                !decl->getName().startswith("k_")) {
                a.report(decl, check_name, "MN03",
                         "Constant member names must begin with 's_' or 'k_'");
            }
        }
        check_pointer(a, decl);
    }
}

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &field_name);
static RegisterCheck c2(check_name, &var_name);

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
