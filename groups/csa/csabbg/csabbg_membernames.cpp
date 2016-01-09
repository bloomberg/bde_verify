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
    if (decl->getAccess() != AS_private) {
        a.report(decl, check_name, "MN01",
                 "Class data members must be private");
    }
}

static void check_pointer(Analyser& a, DeclaratorDecl const *decl)
{
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
    if (decl->isCXXClassMember()) {
        check_private(a, decl);
        if (!decl->getName().startswith("s_")) {
            a.report(decl, check_name, "MN03",
                     "Static data member names must begin with 's_'");
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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
