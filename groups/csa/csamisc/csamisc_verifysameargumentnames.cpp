// csamisc_verifysameargumentnames.cpp                                -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/Redeclarable.h>
#include <clang/Basic/Diagnostic.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <llvm/Support/Casting.h>
#include <algorithm>
#include <string>

namespace clang { class Decl; }

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("verify-same-argument-names");

// -----------------------------------------------------------------------------

namespace
{
bool arg_names_match(ParmVarDecl const* p0, ParmVarDecl const* p1)
{
    std::string n0(p0->getNameAsString());
    std::string n1(p1->getNameAsString());
    bool rc = n0.empty() || n1.empty() || n0 == n1;
    return rc;
}

struct same_argument_names
{
    same_argument_names(Analyser* analyser, FunctionDecl const* current)
    : analyser_(analyser), current_(current)
    {
    }

    void operator()(Decl const* decl)
    {
        if (FunctionDecl const* p = llvm::dyn_cast<FunctionDecl>(decl)) {
            FunctionDecl const* c(current_);
            unsigned n = p->getNumParams();
            if (n == c->getNumParams()) {
                for (unsigned i = 0; i < n; ++i) {
                    const ParmVarDecl* pp = p->getParamDecl(i);
                    const ParmVarDecl* cp = c->getParamDecl(i);
                    if (!arg_names_match(pp, cp)) {
                        auto r1 = analyser_->report(cp->getLocation(),
                                          check_name, "AN01",
                                          "Parameter name mismatch for "
                                          "%ordinal0 parameter %1");
                        r1 << int(i + 1) << cp;
                        auto r2 = analyser_->report(pp->getLocation(),
                                          check_name, "AN01",
                                          "The other declaration uses %0",
                                          false,
                                          DiagnosticIDs::Note);
                        r2 << pp;
                    }
                }
            }
        }
    }

    Analyser* analyser_;
    FunctionDecl const* current_;
};
}

// -----------------------------------------------------------------------------

static void
verify_arg_names_match(Analyser& analyser, FunctionDecl const* decl)
{
    if (!decl->isFirstDecl()) {
        std::for_each(decl->redecls_begin(),
                      decl->redecls_end(),
                      same_argument_names(&analyser, decl));
    }
}

// -----------------------------------------------------------------------------

static RegisterCheck check(check_name, &verify_arg_names_match);

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
