// csamisc_swapab.cpp                                                 -*-C++-*-
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_registercheck.h>

#ident "$Id$"

// ----------------------------------------------------------------------------

static std::string const check_name("swap-a-b");

// ----------------------------------------------------------------------------

using clang::FunctionDecl;
using clang::FunctionTemplateDecl;
using clang::ParmVarDecl;

using bde_verify::csabase::Analyser;

namespace
{

void allFunDecls(Analyser& analyser, const FunctionDecl* func)
    // Callback function for inspecting function declarations.
{
    const ParmVarDecl *pa;
    const ParmVarDecl *pb;

    if (   func->getDeclName().isIdentifier()
        && func->getName() == "swap"
        && func->getNumParams() == 2
        && (pa = func->getParamDecl(0))->getType() ==
           (pb = func->getParamDecl(1))->getType()
        && (   pa->getType()->isPointerType()
            || pa->getType()->isReferenceType()
            )
        ) {
        if (!pa->getName().empty() && pa->getName() != "a") {
            analyser.report(pa->getLocStart(), check_name, "SWAB01",
                            "First parameter of 'swap' should be named 'a'")
                << pa->getSourceRange();
        }
        if (!pb->getName().empty() && pb->getName() != "b") {
            analyser.report(pb->getLocStart(), check_name, "SWAB01",
                            "Second parameter of 'swap' should be named 'b'")
                << pb->getSourceRange();
        }
    }
}
 
void allTpltFunDecls(Analyser& analyser, const FunctionTemplateDecl* func)
    // Callback function for inspecting function template declarations.
{
    allFunDecls(analyser, func->getTemplatedDecl());
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static bde_verify::csabase::RegisterCheck c1(check_name, &allFunDecls);
static bde_verify::csabase::RegisterCheck c2(check_name, &allTpltFunDecls);
