// csabbg_allocatornewwithpointer.cpp                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>
#include <llvm/Support/raw_ostream.h>
#ident "$Id: allocator_new_with_pointer.cpp 141 2011-09-29 18:59:08Z kuehl $"

// -----------------------------------------------------------------------------

static std::string const check_name("allocator-new");

static void
check(cool::csabase::Analyser& analyser, clang::CXXNewExpr const* expr)
{
    if (expr->getNumPlacementArgs() == 1
        && expr->getPlacementArg(0)->getType()->isPointerType())
    {
        clang::Expr* placement(const_cast<clang::Expr*>(expr->getPlacementArg(0))->IgnoreParenImpCasts());
        if (placement && placement->getType()->isPointerType()) {
            clang::QualType pointee(placement->getType()->getPointeeType());
            clang::TypeDecl* bslma_allocator(analyser.lookup_type("::BloombergLP::bslma_Allocator")); 

            if (bslma_allocator
                && bslma_allocator->getTypeForDecl()
                && bslma_allocator->getTypeForDecl()->getCanonicalTypeInternal() == pointee->getCanonicalTypeInternal())
            {
                analyser.report(placement, check_name, "ANP01",
                        "Allocator new with pointer")
                    << placement->getSourceRange();
            }
        }
    }
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_check(check_name, &check);
