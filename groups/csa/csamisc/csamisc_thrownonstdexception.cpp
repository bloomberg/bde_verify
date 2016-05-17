// csamisc_thrownonstdexception.cpp                                   -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Type.h>
#include <clang/Sema/Sema.h>
#include <csabase_analyser.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_registercheck.h>
#include <string>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

static std::string const check_name("throw-non-std-exception");

// -----------------------------------------------------------------------------
//-dk:TODO cache the type of std::exception

static void check(Analyser& analyser, CXXThrowExpr const* expr)
{
    const TypeDecl *e = analyser.lookup_type("::std::exception");
    Expr *object(const_cast<Expr*>(expr->getSubExpr()));
    if (e && e->getTypeForDecl() && object) // else it is a rethrow...
    {
        QualType t = e->getTypeForDecl()->getCanonicalTypeInternal();
        QualType ot = object->getType()->getCanonicalTypeInternal();
        if (ot != t &&
            !analyser.sema().IsDerivedFrom(SourceLocation(), ot, t)) {
            analyser.report(expr, check_name, "FE01",
                            "Object of type %0 not derived from "
                            "std::exception is thrown.")
                << ot;
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
