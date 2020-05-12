// csatr_globaltypeonlyinsource.cpp                                   -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclarationName.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_registercheck.h>
#include <algorithm>
#include <string>

using namespace csabase;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("global-type-only-in-source");

// ----------------------------------------------------------------------------

namespace
{
    struct decl_not_in_toplevel
    {
        decl_not_in_toplevel(Analyser* analyser) : analyser_(analyser)
        {
        }

        bool operator()(Decl const& decl) const
        {
            return (*this)(&decl);
        }

        bool operator()(Decl const* decl) const
        {
            return !analyser_->is_toplevel(
                analyser_->get_location(decl).file());
        }

        Analyser* analyser_;
    };
}

// ----------------------------------------------------------------------------

static void
global_type_only_in_source(Analyser& analyser, TypeDecl const* decl)
{
    if (decl->getDeclContext()->isFileContext()
        && !llvm::dyn_cast<TypedefDecl>(decl)
        && !llvm::dyn_cast<ClassTemplateSpecializationDecl>(decl)
        && !analyser.is_component_header(analyser.toplevel())
        && analyser.is_toplevel(analyser.get_location(decl).file())
        && !decl->isInAnonymousNamespace()
        && !analyser.is_test_driver()
        && decl->isExternallyVisible()
        && decl->getDeclName().isIdentifier()
        && !decl->getDeclName().getAsString().empty()
        && FileName(analyser.get_location(decl).file()).extra() != ".m"
        && FileName(analyser.get_location(decl).file()).extra() != ".g"
        )
    {
        auto r = llvm::dyn_cast<TagDecl>(decl);
        if (r && r->getDefinition() == decl) {
            analyser.report(decl, check_name, "TR11",
                            "Globally visible type '%0' "
                            "should be defined in header.")
                << decl->getQualifiedNameAsString();
        }
        else if (std::find_if(decl->redecls_begin(), decl->redecls_end(),
                              decl_not_in_toplevel(&analyser)) ==
                 decl->redecls_end()) {
            analyser.report(decl, check_name, "TR10",
                            "Globally visible type '%0' "
                            "is not declared in header.")
                << decl->getQualifiedNameAsString();
        }
    }
}

// ----------------------------------------------------------------------------

static RegisterCheck check(check_name, &global_type_only_in_source);

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
