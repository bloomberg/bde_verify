// csabbg_enumvalue.cpp                                               -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("enum-value");

// ----------------------------------------------------------------------------

static void enum_value(Analyser& analyser, EnumDecl const *decl)
{
    if (analyser.is_component(decl) && decl->getNameAsString() == "Value") {
        auto report = analyser.report(decl, check_name, "EV01",
                        "Do not use 'Value' as enum name");
        report << decl->getDeclName();
    }
}

// ----------------------------------------------------------------------------

static RegisterCheck c0(check_name, &enum_value);

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
