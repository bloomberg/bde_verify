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
check(cool::csabase::Analyser& analyser, clang::CXXThrowExpr const* expr)
{
    clang::Sema& sema(analyser.sema());
    clang::Expr* object(const_cast<clang::Expr*>(expr->getSubExpr()));
    if (object) // else it is a rethrow...
    {
        clang::TypeDecl*  exceptionType(analyser.lookup_type("::std::exception"));
        if (!exceptionType || !sema.IsDerivedFrom(object->getType(), exceptionType->getTypeForDecl()->getCanonicalTypeInternal()))
        {
            analyser.report(expr, check_name, "Object of type %0 not derived from std::exception is thrown.")
                << object->getType();
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &check);
