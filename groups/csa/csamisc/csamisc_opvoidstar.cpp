// csamisc_opvoidstar.cpp                                             -*-C++-*-

#include <clang/AST/DeclCXX.h>
#include <clang/AST/Type.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("operator-void-star");

// ----------------------------------------------------------------------------

namespace
{

void conversions(Analyser& analyser, const CXXConversionDecl* conv)
    // Callback function for inspecting conversion declarations.
{
    Location loc(analyser.get_location(conv->getBeginLoc()));
    if (analyser.is_component(loc.file()))
    {
        QualType type = conv->getConversionType();
        if (   !conv->isExplicit()
            && (   type->isBooleanType()
                || (   type->isPointerType()
                    && type->getPointeeType()->isVoidType()
                    )
                )
            ) {
            auto report = analyser.report(conv, check_name, "CB01",
                            "Consider using conversion to "
                            "bsls::UnspecifiedBool<%0>::BoolType instead");
            report << conv->getParent()->getNameAsString();
        }
    }
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &conversions);

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
