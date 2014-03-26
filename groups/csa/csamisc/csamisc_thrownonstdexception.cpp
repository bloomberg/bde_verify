// csamisc_thrownonstdexception.cpp                                   -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <clang/Sema/Sema.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("throw-non-std-exception");

// -----------------------------------------------------------------------------
//-dk:TODO cache the type of std::exception

static void
check(bde_verify::csabase::Analyser& analyser, clang::CXXThrowExpr const* expr)
{
    static const clang::TypeDecl *e = analyser.lookup_type("::std::exception");
    clang::Expr *object(const_cast<clang::Expr*>(expr->getSubExpr()));
    if (e && object) // else it is a rethrow...
    {
        clang::QualType t =
            e->getTypeForDecl()->getCanonicalTypeInternal();
        clang::QualType ot = object->getType()->getCanonicalTypeInternal();
        if (ot != t && !analyser.sema().IsDerivedFrom(ot, t)) {
            analyser.report(expr, check_name, "FE01",
                            "Object of type %0 not derived from "
                            "std::exception is thrown.")
                << ot;
        }
    }
}

// -----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck register_check(check_name, &check);
