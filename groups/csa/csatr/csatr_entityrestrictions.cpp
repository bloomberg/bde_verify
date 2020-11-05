// csatr_entityrestrictions.cpp                                       -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceLocation.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("entity-restrictions");

// ----------------------------------------------------------------------------

static void enum_declaration(Analyser& analyser, EnumDecl const* decl)
{
    if (   decl->getDeclContext()->isFileContext()
        && !analyser.is_global_name(decl)
        && !decl->getLocation().isMacroID()
        && analyser.is_component_header(decl)
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())) {
        auto report = analyser.report(decl, check_name, "TR17",
                        "Enum '%0' declared at global scope");
        report << decl->getName();
    }
}

// ----------------------------------------------------------------------------

static void var_declaration(Analyser& analyser, VarDecl const* decl)
{
    if (   decl->getDeclContext()->isFileContext()
        && !analyser.is_global_name(decl)
        && !decl->getLocation().isMacroID()
        && analyser.is_component_header(decl)
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())) {
        auto report = analyser.report(decl, check_name, "TR17",
                        "Variable '%0' declared at global scope");
        report << decl->getName();
    }
}

// ----------------------------------------------------------------------------

static bool
is_swap(FunctionDecl const* decl)
{
    return decl->getNameAsString() == "swap"
        && decl->getNumParams() == 2
        && decl->getParamDecl(0)->getType()->getCanonicalTypeInternal() ==
           decl->getParamDecl(1)->getType()->getCanonicalTypeInternal()
        && decl->getParamDecl(0)->getType().getTypePtr()->isReferenceType()
        && !decl->getParamDecl(0)->getType().getNonReferenceType()
                .isConstQualified();
}

static void function_declaration(Analyser& analyser, FunctionDecl const* decl)
{
    if (   decl->getDeclContext()->isFileContext()
        && !analyser.is_global_name(decl)
        && !decl->getLocation().isMacroID()
        && analyser.is_component_header(decl)
        && !decl->isOverloadedOperator()
        && !is_swap(decl)
        && decl->getNameAsString() != "debugprint"
        && decl->isFirstDecl()
        && !analyser.is_ADL_candidate(decl)
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())
        ) {
        auto report = analyser.report(decl, check_name, "TR17",
                        "Function '%0' declared at global scope");
        report << decl->getNameAsString()
            << decl->getNameInfo().getSourceRange();
    }
}

// -----------------------------------------------------------------------------

static void typedef_declaration(Analyser& analyser, TypedefDecl const* decl)
{
    // Allow global scope typedef to a name that begins with "package_" for
    // legacy support of "typedef package::name package_name;".
    std::string package = analyser.package() + "_";
    if (package.find("bslfwd_") == 0) {
        package = package.substr(7);
    }
    if (   decl->getDeclContext()->isFileContext()
        && !analyser.is_global_name(decl)
        && !decl->getLocation().isMacroID()
        && analyser.is_component_header(decl)
        && decl->getNameAsString().find(package) != 0
        && !analyser.is_standard_namespace(decl->getQualifiedNameAsString())
        ) {
        auto report = analyser.report(decl, check_name, "TR17",
                        "Typedef '%0' declared at global scope");
        report << decl->getNameAsString();
    }
}

// -----------------------------------------------------------------------------

static RegisterCheck c0(check_name, &enum_declaration);
static RegisterCheck c1(check_name, &var_declaration);
static RegisterCheck c2(check_name, &function_declaration);
static RegisterCheck c3(check_name, &typedef_declaration);

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
